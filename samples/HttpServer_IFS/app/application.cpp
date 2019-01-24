/*
 * Webserver demo using IFS
 *
 */


#include "JsonConfigFile.h"
#include "HardwareSerial.h"
#include "Digital.h"

#if DEBUG_BUILD

extern "C" void __assert_func(const char *file, int line, const char *func, const char *what)
{
  SYSTEM_ERROR("Assertion failed: (%s), function %s, file %s, line %d", what, func, file, line);
  while (1) {
    //
  }
}

#endif


#include "FileManager.h"
static FileManager g_fileManager;
IMPORT_FSTR(fwfsImage, "out/fwfiles.bin")

#include "TimeManager.h"
#include "NetworkManager.h"

#include "WebServer.h"
static WebServer g_webServer;

#include "../include/WebsocketManager.h"

#include "AuthManager.h"
static AuthManager g_authManager;


/*
 * Delay between restart request and actual restart gives time to close network connections,
 * in an order manner so we can notify clients first.
 */
#define RESTART_DELAY_MS  2000

// Time management settings
static DEFINE_FSTR(FILE_TIMEMGMT, ".time.json")

// Some messages only displayed briefly
static unsigned MESSAGE_TIMEOUT = 4000;
// Time to display IP address
static unsigned IPADDR_TIMEOUT = 20000;

//
static HardwareSerial dbgser(UART_ID_1);


// System commands
static DEFINE_FSTR(METHOD_SYSTEM, "system")
static DEFINE_FSTR(COMMAND_RESTART, "restart")
static DEFINE_FSTR(COMMAND_RELOAD, "reload")
// Get System Info
static DEFINE_FSTR(ATTR_LOCAL_TIME, "Local Time")
static DEFINE_FSTR(ATTR_TZ_OFFSET, "TZ Offset")
static DEFINE_FSTR(ATTR_SDK_VERSION, "SDK Version")
static DEFINE_FSTR(ATTR_FREE_HEAP, "Free Heap")
static DEFINE_FSTR(ATTR_CPUFREQ, "CPU Frequency")
static DEFINE_FSTR(ATTR_CHIP_ID, "Chip ID")
static DEFINE_FSTR(ATTR_FLASH_ID, "Flash ID")
static DEFINE_FSTR(ATTR_COMPILE_DATETIME, "Compiled")
static DEFINE_FSTR(ATTR_MAX_TASK_COUNT, "Max Tasks");
static DEFINE_FSTR(COMPILE_DATETIME, __DATE__ " " __TIME__)


class CSystemCommands: public ICommandHandler
{
  public:
    static void restart();


    /* CCommandHandler */

    String getMethod() const
    {
      return METHOD_SYSTEM;
    }

    UserRole minAccess() const
    {
      return UserRole::Admin;
    }

    void handleMessage(command_connection_t connection, JsonObject& json);
};

static CSystemCommands g_system;


/*
 * Schedule a system restart.
 *
 * This sometimes doesn't work and system hangs. Seems to happen
 * only when debug interface is connected so likely related to
 * GPIO pins being in wrong state.
 */
void CSystemCommands::restart()
{
#ifdef USERIO_ENABLE
  g_userio.setMessage(DISP_SHUTDOWN);
#endif

  DynamicJsonBuffer buffer;
  JsonObject& json = buffer.createObject();
  json[ATTR_METHOD] = String(METHOD_SYSTEM);
  json[ATTR_COMMAND] = String(COMMAND_RESTART);
  setPending(json);
  WSCommandConnection::broadcast(json);

  g_webServer.stop();

  SimpleTimer* restartTimer = new SimpleTimer;
  restartTimer->setCallback([](void* timer) {
    delete static_cast<SimpleTimer*>(timer);
    WifiStation.disconnect();
    System.restart();
  }, restartTimer);
  restartTimer->startMs(RESTART_DELAY_MS);
}


void CSystemCommands::handleMessage(command_connection_t connection, JsonObject& json)
{
  const char* command = json[ATTR_COMMAND];

  if (COMMAND_INFO == command) {
	json[ATTR_LOCAL_TIME] = SystemClock.getSystemTimeString(eTZ_Local);
	json[ATTR_TZ_OFFSET] = SystemClock.getTimeZoneOffset();
    json[ATTR_SDK_VERSION] = system_get_sdk_version();
    json[ATTR_SDK_VERSION] = system_get_sdk_version();
    json[ATTR_FREE_HEAP] = system_get_free_heap_size();
    json[ATTR_CPUFREQ] = system_get_cpu_freq();
    json[ATTR_CHIP_ID] = system_get_chip_id();
    json[ATTR_FLASH_ID] = spi_flash_get_id();
    json[ATTR_COMPILE_DATETIME] = String(COMPILE_DATETIME);
    json[ATTR_MAX_TASK_COUNT] = String(System.getMaxTaskCount());
    setSuccess(json);
    return;
  }

  if (COMMAND_RESTART == command) {
    restart();
    json[DONT_RESPOND] = true;
    return;
  }

  ICommandHandler::handleMessage(connection, json);
}


static void networkStatusChanged(network_change_t nwc)
{
}




static void systemReady()
{
  {
    JsonConfigFile config;
    if (config.load(FILE_TIMEMGMT))
      timeManager.configure(config);
    timeManager.onChange([](int adjustSecs) {
      // System time has been updated, clock adjusted by specified amount
    });
  }

  socketManager.registerHandler(g_fileManager);
  g_fileManager.onUpload([](const FileUpload& upload) {
	  // If user uploads a file we may need to trigger a refresh, for examnple if it's a config file
  });

  socketManager.registerHandler(networkManager);

  socketManager.registerHandler(g_authManager);
  g_authManager.onLoginComplete(login_callback_t(&WebsocketManager::loginComplete, &socketManager));

  socketManager.registerHandler(g_webServer);

  socketManager.registerHandler(g_system);

  g_webServer.start();
}


void init()
{
  // Configure serial ports
//  Serial.systemDebugOutput(false);
  Serial.end();

#if DEBUG_BUILD
  dbgser.setTxBufferSize(4096);
  dbgser.setRxBufferSize(0);
//  dbgser.setTxWait(false);
  dbgser.begin(COM_SPEED_SERIAL);

  dbgser.systemDebugOutput(true);
  debug_i("\n\n********************************************************\n"
          "Hello\n");
#endif

  LOAD_FSTR(dt, COMPILE_DATETIME)
  debug_i("Firmware compiled %s", dt);

  // Ensure pins don't float
  pinMode(1, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);

  if (!g_fileManager.init(fwfsImage.flashData)) {
	  // Eeek! We're screwed
	  debug_e("File init failed");
  }

  System.onReady(systemReady);

  networkManager.onStatusChange(networkStatusChanged);
  networkManager.begin();
}

