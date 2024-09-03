
#include <linux/init.h>
#include <linux/module.h>
#include <linux/serdev.h>
#include <linux/of_device.h>
#include <linux/completion.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>

#define GROVE_WIFI_DRIVER_NAME "grove_wifi"

#define RESPONSE_OK "OK"
#define RESPONSE_ERROR "ERROR"
#define RESPONSE_FAIL "FAIL"
#define GROVE_CMD_MAX_LENGTH 60
#define GROVEWIFI_RSP_MAX_LENGTH 35
#define GROVEWIFI_RSP_RAW_MAX_LENGTH 200
#define STD_TIMEOUT 3*HZ
#define EXT_TIMEOUT 10*HZ

static char cmd_cli[GROVE_CMD_MAX_LENGTH];
static unsigned int cli_id;
static kobject *kern_obj;

enum grove_wifi_cli {
    CLI_CHK_STATE,
    CLI_CONNECT,
    CLI_DISCONNECT,
    CLI_PING,
    CLI_GET_IP,
};

enum grove_wifi_cmd {
	CMD_TEST,
	CMD_NO_ECHO,
	CMD_DISABLE_SLEEP,
	CMD_SET_UART,
	CMD_STATION_MODE,
    CMD_CLI,
};

enum grove_wifi_comm_state{
    OK,
    ERROR,
    NEUTRAL,
};

struct grove_wifi_state {
    struct serdev_device *serdev;
    struct completion cmd_done;
    struct mutex lock;
    char response[GROVEWIFI_RSP_RAW_MAX_LENGTH];
    size_t response_len;
    enum grove_wifi_comm_state comm;
};

struct grove_wifi_state *state_global;

static u8 grove_wifi_cmd_tbl[][GROVE_CMD_MAX_LENGTH] = {
	[CMD_TEST] = "\r\nAT\r\n",
	[CMD_NO_ECHO] = "\r\nATE0\r\n",
	[CMD_DISABLE_SLEEP] = "\r\nAT+SLEEP=0\r\n",
	[CMD_SET_UART] = "\r\nAT+UART_CUR=115200,8,1,0,0\r\n",
	[CMD_STATION_MODE] = "\r\nAT+CWMODE=1\r\n",
    [CMD_CLI] = "\r\n\r\n",
};

/******************************************************************************************************************/
/*********************************************** Headers **********************************************************/
/******************************************************************************************************************/

static int grove_wifi_sysfs_setup(struct serdev_device *serdev);
void grove_wifi_clear_frame (struct grove_wifi_state *state);
static int grove_wifi_do_cmd(struct grove_wifi_state *state, enum grove_wifi_cmd cmd);

/******************************************************************************************************************/
/******************************************************************************************************************/

/******************************************************************************************************************/
/************************************* CLI Id Entry Setup **********************************************************/
/******************************************************************************************************************/

static ssize_t grove_wifi_fct_id_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    sscanf(buf, "%du", &cli_id);
    return count;
}

static ssize_t grove_wifi_fct_id_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
    pr_alert("Operation not allowed: cli file is Write-only\n");
    return -EPERM;
}

static struct kobj_attribute id =
    __ATTR(cli_id, 0644, grove_wifi_fct_id_show, grove_wifi_fct_id_store);

/******************************************************************************************************************/
/************************************* CLI Entry Setup **********************************************************/
/******************************************************************************************************************/

static ssize_t grove_wifi_cli_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int ret;
    struct serdev_device *serdev = state_global->serdev;

    sprintf(grove_wifi_cmd_tbl[CMD_CLI], "\r\n%s\r\n", buf);

    ret = grove_wifi_do_cmd(state_global, CMD_CLI);
	
   	if (ret < 0) {
        strcpy(grove_wifi_cmd_tbl[CMD_CLI], "\r\n\r\n");
    	return -EIO;
    }

    strcpy(grove_wifi_cmd_tbl[CMD_CLI], "\r\n\r\n");
    return count;
}

static ssize_t grove_wifi_cli_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
    pr_alert("Operation not allowed: cli file is Write-only\n");
    return -EPERM;
}

static struct kobj_attribute cli =
    __ATTR(cmd_cli, 0644, grove_wifi_cli_show, grove_wifi_cli_store);

/******************************************************************************************************************/
/************************************** RESPONSE Entry Setup **********************************************************/
/******************************************************************************************************************/

static ssize_t grove_wifi_response_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    pr_alert("Operation not allowed: cli file is Write-only\n");
    return -EPERM;
}

static ssize_t grove_wifi_response_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
    int ret;
    enum grove_wifi_comm_state comm_state = state_global->comm;
    char response_cli[GROVEWIFI_RSP_MAX_LENGTH];

    switch (comm_state)
    {
    case ERROR:
        strcpy(response_cli,"Unsuccessful Operation");
        break;
    
    case OK:
        strcpy(response_cli,"Successful operation");
        break;
        
    default:
        strcpy(response_cli,"Waiting for response");
        break;
    }

    return sprintf(buf, "%s", response_cli);
    
}

static struct kobj_attribute response =
    __ATTR(response_cli, 0644, grove_wifi_response_show, grove_wifi_response_store);

/******************************************************************************************************************/
/******************************************************************************************************************/


static int grove_wifi_sysfs_setup(struct serdev_device *serdev){
    struct grove_wifi_state *state = serdev_device_get_drvdata(serdev);
    int error;
    kern_obj = kobject_create_and_add("grovewifiv2", kernel_kobj); 
    
    if ( !kern_obj )
        return -ENOMEM;
    
    error = sysfs_create_file(kern_obj, &cli.attr);
    if (error) {
        dev_err(&serdev->dev, "CLI file not created under /sys/kernel/grovewifiv2\n");
        return -ENOMEM;
    }
    error = sysfs_create_file(kern_obj, &response.attr);
    if (error) {
        dev_err(&serdev->dev, "RESPONSE file not created under /sys/kernel/grovewifiv2\n");
        return -ENOMEM;
    }
    error = sysfs_create_file(kern_obj, &id.attr);
    if (error) {
        dev_err(&serdev->dev, "ID file not created under /sys/kernel/grovewifiv2\n");
        return -ENOMEM;
    }

    dev_info(&serdev->dev, "Successfully created /sys/kernel/grovewifiv2\n");
    return 0;
}


/******************************************************************************************************************/
/******************************************************************************************************************/

/******************************************************************************************************************/
/************************************** Kernel Level Functions ****************************************************/
/******************************************************************************************************************/

static int grove_wifi_receive_buf(struct serdev_device *serdev, const u8 *buf, size_t size) {
    struct grove_wifi_state *state = serdev_device_get_drvdata(serdev);
    size_t num;

    mutex_lock(&state->lock);

    num = min(size, sizeof(state->response) - state->response_len - 1);
    memcpy(state->response + state->response_len, buf, num);
    state->response_len += num;
    state->response[state->response_len] = '\0';
    if (strstr(state->response, RESPONSE_OK)) { 
        dev_info(&serdev->dev, "Command successfully executed");
        complete(&state->cmd_done);
    }
    
    mutex_unlock(&state->lock);

    return (int) size;
}

static const struct serdev_device_ops grove_wifi_serdev_ops = {
    .receive_buf = grove_wifi_receive_buf,
    .write_wakeup = serdev_device_write_wakeup,
};

void grove_wifi_clear_frame (struct grove_wifi_state *state)
{
	mutex_lock(&state->lock);
	strcpy(state->response,"");
	state->response_len = 0;
    state->comm = NEUTRAL;
	mutex_unlock(&state->lock);
}

static int grove_wifi_do_cmd(struct grove_wifi_state *state, enum grove_wifi_cmd cmd) //TODO Response "OK"
{
	struct serdev_device *serdev = state->serdev;
	int ret;

    grove_wifi_clear_frame(state);
	
    mutex_lock(&state->lock);
	ret = serdev_device_write(serdev, grove_wifi_cmd_tbl[cmd], strlen(grove_wifi_cmd_tbl[cmd]), STD_TIMEOUT);
	mutex_unlock(&state->lock);

	if (ret < strlen(grove_wifi_cmd_tbl[cmd]))
		return ret < 0 ? ret : -EIO;

	ret = wait_for_completion_timeout(&state->cmd_done, EXT_TIMEOUT);
	if (!ret){
		dev_err(&serdev->dev, "Timeout waiting for response\n");
        reinit_completion(&state->cmd_done);
        state->comm = ERROR;
		return -ETIMEDOUT;
	}

    state->comm = OK;
    reinit_completion(&state->cmd_done);
	return 0;
}

static int grove_wifi_probe(struct serdev_device *serdev) {
    struct grove_wifi_state *state;
    int ret;

    state = devm_kzalloc(&serdev->dev, sizeof(*state), GFP_KERNEL);
    state_global = state;
    
    if (!state)
        return -ENOMEM;

    serdev_device_set_drvdata(serdev, state);
    state->serdev = serdev;
    mutex_init(&state->lock);
    init_completion(&state->cmd_done);

    serdev_device_set_client_ops(serdev, &grove_wifi_serdev_ops);
    ret = devm_serdev_device_open(&serdev->dev, serdev);
    if (ret)
        return ret;

    serdev_device_set_baudrate(serdev, 115200);
    serdev_device_set_flow_control(serdev, false);
    ret = serdev_device_set_parity(serdev, SERDEV_PARITY_NONE);
    if (ret)
        return ret;
	
	for ( enum grove_wifi_cmd cmd = CMD_TEST; cmd <= CMD_STATION_MODE; cmd++ ){
		ret = grove_wifi_do_cmd(state, cmd);
    	if (ret < 0){
            dev_err(&serdev->dev, "Error occured, unexpected response\n");
        	return -EIO;
        }
	}

    dev_info(&serdev->dev, "Grove WiFi module is configured\n");
    grove_wifi_sysfs_setup (serdev);

    return 0;
}

static const struct of_device_id grove_wifi_of_match[] = {
    { .compatible = "seeedstudio,grovewifiv1" },
	{ .compatible = "seeedstudio,grovewifiv2" },
	{ }
};
MODULE_DEVICE_TABLE(of, grove_wifi_of_match);

static struct serdev_device_driver grove_wifi_driver = {
    .driver = {
        .name = GROVE_WIFI_DRIVER_NAME,
        .of_match_table = of_match_ptr(grove_wifi_of_match),
    },
    .probe = grove_wifi_probe,
};
module_serdev_device_driver(grove_wifi_driver);

MODULE_LICENSE("GPL v3");
MODULE_AUTHOR("AKhadhraoui47");
MODULE_DESCRIPTION("Grove Wifi Kernel Module");

/******************************************************************************************************************/
/******************************************************************************************************************/

