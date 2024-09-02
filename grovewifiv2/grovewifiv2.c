#include <linux/init.h>
#include <linux/module.h>
#include <linux/serdev.h>
#include <linux/of_device.h>
#include <linux/completion.h>
#include <linux/mutex.h>

#define GROVE_WIFI_DRIVER_NAME "grove_wifi"
#define AT_CMD "\r\nAT\r\n"
#define RESPONSE_OK "\r\nOK\r\n"

struct grove_wifi_state {
    struct serdev_device *serdev;
    struct completion cmd_done;
    struct mutex lock;
    char response[64];
    size_t response_len;
};

static int grove_wifi_receive_buf(struct serdev_device *serdev, const u8 *buf, size_t size) {
    struct grove_wifi_state *state = serdev_device_get_drvdata(serdev);
    size_t num;

    mutex_lock(&state->lock);

    num = min(size, sizeof(state->response) - state->response_len - 1);
    memcpy(state->response + state->response_len, buf, num);
    state->response_len += num;
    state->response[state->response_len] = '\0';

    if (strstr(state->response, RESPONSE_OK)) {
        complete(&state->cmd_done);
    }

    mutex_unlock(&state->lock);

    return (int)size;
}

static const struct serdev_device_ops grove_wifi_serdev_ops = {
    .receive_buf = grove_wifi_receive_buf,
    .write_wakeup = serdev_device_write_wakeup,
};

static int grove_wifi_probe(struct serdev_device *serdev) {
    struct grove_wifi_state *state;
    int ret;

    state = devm_kzalloc(&serdev->dev, sizeof(*state), GFP_KERNEL);
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

    mutex_lock(&state->lock);
    ret = serdev_device_write(state->serdev, AT_CMD, strlen(AT_CMD), HZ);
    mutex_unlock(&state->lock);

    if (ret < 0)
        return ret;

    ret = wait_for_completion_timeout(&state->cmd_done, HZ);
    if (!ret) {
        dev_err(&serdev->dev, "Timeout waiting for response\n");
        return -ETIMEDOUT;
    }

    if (!strstr(state->response, RESPONSE_OK)) {
        dev_err(&serdev->dev, "Unexpected response: %s\n", state->response);
        return -EIO;
    }

    dev_info(&serdev->dev, "Grove WiFi module responded with OK\n");

	/****************************************/
	
	init_completion(&state->cmd_done);
	mutex_lock(&state->lock);
    ret = serdev_device_write(state->serdev, AT_CMD, strlen(AT_CMD), HZ);
    mutex_unlock(&state->lock);

    if (ret < 0)
        return ret;

    ret = wait_for_completion_timeout(&state->cmd_done, HZ);
    if (!ret) {
        dev_err(&serdev->dev, "Timeout waiting for response\n");
        return -ETIMEDOUT;
    }

    if (!strstr(state->response, RESPONSE_OK)) {
        dev_err(&serdev->dev, "Unexpected response: %s\n", state->response);
        return -EIO;
    }

    dev_info(&serdev->dev, "Grove WiFi module responded with OK\n");

	/****************************************/

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

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("AKhadhraoui47");
MODULE_DESCRIPTION("Grove Wifi V2 Kernel Module");
