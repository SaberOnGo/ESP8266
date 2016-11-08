/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Martin d'Allens <martin.dallens@gmail.com> wrote this file. As long as you retain
 * this notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return.
 * ----------------------------------------------------------------------------
 */

#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"
#include "httpclient.h"

#include "AirKiss_Drv.h"

static int test_count = 0;
static ETSTimer test_timer;

static void ICACHE_FLASH_ATTR test_timer_cb(void *arg)
{
	HTTP_DEBUG("\nTEST %d ", test_count);

	switch(test_count) {
#if 0
	case 0:
		HTTP_DEBUG("=> IP address before network ready\n");
		http_get("http://173.194.45.65", "", http_callback_example);
		// This test will fail. The DHCP request returns directly, but the TCP connection hangs.
		// FIXME: wait for network to be ready before connecting?
		break;
	case 1:
		HTTP_DEBUG("=> Simple GET\n");
		http_get("http://wtfismyip.com/text", "", http_callback_example);
		break;
	case 2:
		HTTP_DEBUG("=> Response too long\n");
		http_get("http://www.wikipedia.org/", "", http_callback_example);
		break;
	case 3:
		HTTP_DEBUG("=> Connection refused\n");
		http_get("http://att.com:1234/", "", http_callback_example);
		break;
	case 4:
		HTTP_DEBUG("=> Empty response, wrong protocol\n");
		http_get("http://google.com:443/", "", http_callback_example);
		break;
	case 5:
		HTTP_DEBUG("=> Invalid DNS\n");
		http_get("http://invalid.dns/", "", http_callback_example);
		break;
	case 6:
		HTTP_DEBUG("=> Connection time out\n");
		http_get("http://google.com:1234/", "", http_callback_example);
		break;
	case 7:
		HTTP_DEBUG("=> Simple POST\n");
		http_post("http://httpbin.org/post", "first_word=hello&second_word=world", "Content-Type: application/x-www-form-urlencoded\r\n", http_callback_example);
		break;
	case 8:
		HTTP_DEBUG("=> Moved\n");
		http_get("http://wikipedia.org/", "", http_callback_example);
		break;
	case 9:
		HTTP_DEBUG("=> IP address, 404\n");
		http_get("https://www.baidu.com", "", http_callback_example);
		break;
#endif

	case 2:
		#if 0
		HTTP_DEBUG("=> Concurrent requests\n");
		http_get("http://wtfismyip.com/text", "", http_callback_example);
		http_post("http://httpbin.org/post", "first_word=hello&second_word=world", "Content-Type: application/x-www-form-urlencoded\r\n", http_callback_example);
		http_get("http://wtfismyip.com/text", "", http_callback_example);
		http_post("http://httpbin.org/post", "first_word=hello&second_word=world", "Content-Type: application/x-www-form-urlencoded\r\n", http_callback_example);
		http_get("http://wtfismyip.com/text", "", http_callback_example);
		http_post("http://httpbin.org/post", "first_word=hello&second_word=world", "Content-Type: application/x-www-form-urlencoded\r\n", http_callback_example);
		http_get("http://wtfismyip.com/text", "", http_callback_example);
		#endif
        HTTP_DEBUG("get weather start\n");
		http_get("https://api.caiyunapp.com/v2/Sw5xKL3A7dfJ44Mo/121.6544,25.1552/realtime.json", "", http_callback_example);
		// FIXME: this test sometimes fails with "DNS error code -1"
		break;
	case 4:
	  {
		HTTP_DEBUG("=> DONE\n");
		http_get("https://api.caiyunapp.com/v2/Sw5xKL3A7dfJ44Mo/121.6544,25.1552/realtime.json", "", http_callback_example);
	  }
	  break;
	default:
	  break;
	}
	test_count++;
	if(test_count > 4)
	{
	   test_count = 0;
	}
	os_timer_arm(&test_timer, 12000, 0);
}

void ICACHE_FLASH_ATTR http_test(void)
{
	// FIXME: what happens when no Wifi network is available?
	os_timer_disarm(&test_timer);
	os_timer_setfn(&test_timer, test_timer_cb, NULL);
	os_timer_arm(&test_timer, 0, 0); // Start immediately.
}

static os_timer_t tTestHttpTimer;
void FLASH_SAVE HttpClient_TestInit(void *arg)
{
    if(AirKiss_IsNetConnected())
    {
       os_timer_disarm(&tTestHttpTimer);
       HTTP_DEBUG("http test run\n");
       http_test();
    }
	else
	{
	   os_timer_disarm(&tTestHttpTimer);
	   os_timer_setfn(&tTestHttpTimer, HttpClient_TestInit, NULL);
	   os_timer_arm(&tTestHttpTimer, 2000, 0);
	}
}
