/*
;    Project:       Open Vehicle Monitor System
;    Date:          14th March 2017
;
;    Changes:
;    1.0  Initial release
;
;    (C) 2011       Michael Stegen / Stegen Electronics
;    (C) 2011-2017  Mark Webb-Johnson
;    (C) 2011        Sonny Chen @ EPRO/DX
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in
; all copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
; THE SOFTWARE.
*/

#include "ovms_log.h"
static const char *TAG = "version";

#include <esp_system.h>
#include <esp_ota_ops.h>
#include "ovms.h"
#include "ovms_metrics.h"
#include "metrics_standard.h"
#include "ovms_events.h"

void Version(std::string event, void* data)
  {
  std::string version(OVMS_VERSION);

  ESP_LOGI(TAG, "Set version");

  const esp_partition_t *p = esp_ota_get_running_partition();
  if (p != NULL)
    {
    version.append("/");
    version.append(p->label);
    }
  version.append("/");
  version.append(CONFIG_OVMS_VERSION_TAG);

  version.append(" build (idf ");
  version.append(esp_get_idf_version());
  version.append(") ");
  version.append(__DATE__);
  version.append(" ");
  version.append(__TIME__);
  StandardMetrics.ms_m_version->SetValue(version.c_str());

  std::string hardware("OVMS ");
  esp_chip_info_t chip;
  esp_chip_info(&chip);
  if (chip.features & CHIP_FEATURE_EMB_FLASH) hardware.append("EMBFLASH ");
  if (chip.features & CHIP_FEATURE_WIFI_BGN) hardware.append("WIFI ");
  if (chip.features & CHIP_FEATURE_BLE) hardware.append("BLE ");
  if (chip.features & CHIP_FEATURE_BT) hardware.append("BT ");
  char buf[32]; sprintf(buf,"cores=%d ",chip.cores); hardware.append(buf);
  sprintf(buf,"rev=ESP32/%d",chip.revision); hardware.append(buf);
  StandardMetrics.ms_m_hardware->SetValue(hardware.c_str());

  uint8_t mac[6];
  esp_efuse_mac_get_default(mac);
  sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",
          mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
  StandardMetrics.ms_m_serial->SetValue(buf);
  }

class VersionInit
  {
  public: VersionInit();
} MyVersionInit  __attribute__ ((init_priority (9900)));

VersionInit::VersionInit()
  {
  ESP_LOGI(TAG, "Initialising Versioning (9900)");

  #undef bind  // Kludgy, but works
  using std::placeholders::_1;
  using std::placeholders::_2;
  MyEvents.RegisterEvent(TAG,"system.start", std::bind(&Version, _1, _2));
  }