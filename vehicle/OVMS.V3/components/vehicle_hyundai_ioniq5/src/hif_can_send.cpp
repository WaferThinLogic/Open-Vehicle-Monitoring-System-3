/*
;    Project:       Open Vehicle Monitor System
;    Date:          September 2022
;
;   (C) 2022 Michael Geddes <frog@bunyip.wheelycreek.net>
; ----- Kona/Kia Module -----
;    (C) 2019       Geir Øyvind Vælidalo <geir@validalo.net>
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
#include "vehicle_hyundai_ioniq5.h"
#include "kia_common.h"
// #define QUIET_WHILE_SENDING

#ifdef XIQ_CAN_WRITE
/**
 * Send a can message and wait for the response before continuing.
 * Time out after approx 0.5 second.
 */
bool OvmsHyundaiIoniqEv::SendCanMessage_sync(uint16_t id, uint8_t count,
  uint8_t serviceId, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4,
  uint8_t b5, uint8_t b6)
{
  XARM("OvmsHyundaiIoniqEv::SendCanMessage_sync");
  
  // Safety check: require intercept harness mode for CAN writes
  if (m_con_obd) {
    ESP_LOGE(TAG, "CAN write blocked: not in intercept harness mode (set xiq.con_obd=0)");
    XDISARM;
    return false;
  }
  
  if (!kia_enable_write) {
    ESP_LOGE(TAG, "CAN write blocked: canwrite not enabled (set xiq.canwrite=1)");
    XDISARM;
    return false;
  }
  
  uint16_t timeout;

#ifdef QUIET_WHILE_SENDING
  int poll = PollGetState();
  PollState_Off();
#endif
  vTaskDelay(xDelay);
  kia_send_can.id = id;
  kia_send_can.status = 0xff;

  uint8_t data[] = {count, serviceId, b1, b2, b3, b4, b5, b6};
  m_can1->WriteStandard(id, 8, data);
  ESP_LOGV(TAG, "Send Sync: %03x n=%02x svc=%02x %02x %02x %02x %02x %02x %02x", id, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);

  //Now wait for the response
  timeout = 50; // 50*50 = 2500 ms
  do {
    /* Block for 50ms. */
    vTaskDelay( xDelay );
  }
  while (kia_send_can.status == 0xff && --timeout > 0);
#ifdef QUIET_WHILE_SENDING
  // Restore poll state
  PollSetState(poll);
#endif
  //Did we get the response?
  if (timeout != 0 ) {
    if ( kia_send_can.byte[1] == serviceId + 0x40 ) {
      ESP_LOGV(TAG, "SendSync: %03x Received %02x %02x %02x %02x %02x %02x %02x %02x",
        id,
        kia_send_can.byte[0],  kia_send_can.byte[1],  kia_send_can.byte[2],  kia_send_can.byte[3],
        kia_send_can.byte[4],  kia_send_can.byte[5],  kia_send_can.byte[6],  kia_send_can.byte[7]);

      XDISARM;
      return true;
    }
    else if ( kia_send_can.byte[1] == 0x7f ) {
      ESP_LOGE(TAG, "SendSync: %03x Failed (unsupported?) [[ %02x %02x %02x %02x %02x %02x %02x %02x]]",
        id,
        kia_send_can.byte[0],  kia_send_can.byte[1],  kia_send_can.byte[2],  kia_send_can.byte[3],
        kia_send_can.byte[4],  kia_send_can.byte[5],  kia_send_can.byte[6],  kia_send_can.byte[7]);
      kia_send_can.status = 0x2;
    }
    else {
      ESP_LOGD(TAG, "SendSync: %03x Mismatch Exp=%02x Rec=%02x [[ %02x %02x %02x %02x %02x %02x %02x %02x]]",
        id, serviceId + 0x40, kia_send_can.byte[1],
        kia_send_can.byte[0],  kia_send_can.byte[1],  kia_send_can.byte[2],  kia_send_can.byte[3],
        kia_send_can.byte[4],  kia_send_can.byte[5],  kia_send_can.byte[6],  kia_send_can.byte[7]);
      kia_send_can.status = 0x2;
    }
  }
  else {
    ESP_LOGD(TAG, "SendSync: Timed Out %03x 8", id);
    kia_send_can.status = 0x1; //Timeout
  }
  XDISARM;
  return false;
}

void OvmsHyundaiIoniqEv::SendCanMessage(uint16_t id, uint8_t count,
  uint8_t serviceId, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4,
  uint8_t b5, uint8_t b6)
{
  XARM("OvmsHyundaiIoniqEv::SendCanMessage");
  
  // Safety check: require intercept harness mode for CAN writes
  if (m_con_obd) {
    ESP_LOGE(TAG, "CAN write blocked: not in intercept harness mode (set xiq.con_obd=0)");
    XDISARM;
    return;
  }
  
  if (!kia_enable_write) {
    ESP_LOGE(TAG, "CAN write blocked: canwrite not enabled (set xiq.canwrite=1)");
    XDISARM;
    return;
  }

  uint8_t data[] = {count, serviceId, b1, b2, b3, b4, b5, b6};
  m_can1->WriteStandard(id, 8, data);
  ESP_LOGV(TAG, "Send Can Msg: %03x n=%02x svc=%02x %02x %02x %02x %02x %02x %02x", id, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  XDISARM;
}

/**
 * Sends same message three times
 */
void OvmsHyundaiIoniqEv::SendCanMessageTriple(uint16_t id, uint8_t count,
  uint8_t serviceId, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4,
  uint8_t b5, uint8_t b6)
{
  XARM("OvmsHyundaiIoniqEv::SendCanMessageTriple");
  
  // Safety check: require intercept harness mode for CAN writes
  if (m_con_obd) {
    ESP_LOGE(TAG, "CAN write blocked: not in intercept harness mode (set xiq.con_obd=0)");
    XDISARM;
    return;
  }
  
  if (!kia_enable_write) {
    ESP_LOGE(TAG, "CAN write blocked: canwrite not enabled (set xiq.canwrite=1)");
    XDISARM;
    return;
  }

  uint8_t data[] = {count, serviceId, b1, b2, b3, b4, b5, b6};
  m_can1->WriteStandard(id, 8, data);
  vTaskDelay( xDelay );
  m_can1->WriteStandard(id, 8, data);
  vTaskDelay( xDelay );
  m_can1->WriteStandard(id, 8, data);
  ESP_LOGV(TAG, "Send Triple: %03x 8 n=%02x svc=%02x %02x %02x %02x %02x %02x %02x", id, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
  XDISARM;
}

/**
 *  Turns on/off head light delay
 */
void OvmsHyundaiIoniqEv::SetHeadLightDelay(bool on)
{
  XARM("OvmsHyundaiIoniqEv::SetHeadLightDelay");
  if (!m_con_obd && kia_enable_write) {
    SendCanMessageTriple(0x014, 0, 0, on ? 0x08 : 0x04, 0, 0, 0, 0, 0);
    ESP_LOGI(TAG, "Head light delay: %s", on ? "enabled" : "disabled");
  }
  XDISARM;
}

/**
 * Set one touch turn signal
 * 0 = off
 * 1 = 3 blink
 * 2 = 5 blink
 * 3 = 7 blink
 *
 */
void OvmsHyundaiIoniqEv::SetOneThouchTurnSignal(uint8_t value)
{
  XARM("OvmsHyundaiIoniqEv::SetOneThouchTurnSignal");
  if (!m_con_obd && kia_enable_write) {
    SendCanMessageTriple(0x014, (value + 1) << 5, 0, 0, 0, 0, 0, 0, 0);
    ESP_LOGI(TAG, "One touch turn signal set to: %d blinks", value * 2 + 1);
  }
  XDISARM;
}

/**
 * Set Auto Door unlock setting
 * 1 = Off
 * 2 = Vehicle Off
 * 3 = On shift to P
 * 4 = Driver door unlock
 */
void OvmsHyundaiIoniqEv::SetAutoDoorUnlock(uint8_t value)
{
  XARM("OvmsHyundaiIoniqEv::SetAutoDoorUnlock");
  if (!m_con_obd && kia_enable_write) {
    SendCanMessageTriple(0x014, 0, value, 0, 0, 0, 0, 0, 0);
    ESP_LOGI(TAG, "Auto door unlock setting: %d", value);
  }
  XDISARM;
}

/**
 * Set Auto Door lock setting
 * 0 = Off
 * 1 = Enable on speed
 * 2 = Enable on Shift
 */
void OvmsHyundaiIoniqEv::SetAutoDoorLock(uint8_t value)
{
  XARM("OvmsHyundaiIoniqEv::SetAutoDoorLock");
  if (!m_con_obd && kia_enable_write) {
    SendCanMessageTriple(0x014, 0, (value + 1) << 5, 0, 0, 0, 0, 0, 0);
    ESP_LOGI(TAG, "Auto door lock setting: %d", value);
  }
  XDISARM;
}

/**
 * Send tester present messages for all ECUs
 */
void OvmsHyundaiIoniqEv::SendTesterPresentMessages()
{
  XARM("OvmsHyundaiIoniqEv::SendTesterPresentMessages");
  if ( iq_shift_status != IqShiftStatus::Park ) {
    StopTesterPresentMessages();
  }
  if ( igmp_tester_present_seconds > 0 ) {
    igmp_tester_present_seconds--;
    SendTesterPresent(INTEGRATED_GATEWAY, 2);
  }
  if ( bcm_tester_present_seconds > 0 ) {
    bcm_tester_present_seconds--;
    SendTesterPresent(BODY_CONTROL_MODULE, 2);
  }
  XDISARM;
}

/**
 * Stop all tester present massages for all ECUs
 */
void OvmsHyundaiIoniqEv::StopTesterPresentMessages()
{
  XARM("OvmsHyundaiIoniqEv::StopTesterPresentMessages");
  igmp_tester_present_seconds = 0;
  bcm_tester_present_seconds = 0;
  XDISARM;
}


/**
 * Send a can message to ECU to notify that tester is present
 */
void OvmsHyundaiIoniqEv::SendTesterPresent(uint16_t id, uint8_t length)
{
  XARM("OvmsHyundaiIoniqEv::SendTesterPresent");
  if (kia_enable_write) {
    SendCanMessage(id, length, UDS_SID_TESTER_PRESENT, 0, 0, 0, 0, 0, 0);
  }
  XDISARM;
}

/**
 * Send a can message to set ECU in a specific session mode
 */
bool OvmsHyundaiIoniqEv::SetSessionMode(uint16_t id, uint8_t mode)
{
  XARM("OvmsHyundaiIoniqEv::SetSessionMode");
  ESP_LOGV(TAG, "Set Session Mode %03x --> %02x", id, mode);
  bool res = SendCanMessage_sync(id, 2, VEHICLE_POLL_TYPE_OBDIISESSION, mode, 0, 0, 0, 0, 0);
  XDISARM;
  return  res;
}

/**
 * Put the car in proper session mode and then send the command
 */
bool OvmsHyundaiIoniqEv::SendCommandInSessionMode(uint16_t id, uint8_t count, uint8_t serviceId, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t mode )
{
  XARM("OvmsHyundaiIoniqEv::SendCommandInSessionMode");
  
  // Safety check: require intercept harness mode for CAN writes
  if (m_con_obd) {
    ESP_LOGE(TAG, "CAN write blocked: not in intercept harness mode (set xiq.con_obd=0)");
    XDISARM;
    return false;
  }
  
  if (!kia_enable_write) {
    ESP_LOGE(TAG, "CAN write blocked: canwrite not enabled (set xiq.canwrite=1)");
    XDISARM;
    return false;
  }

  bool result = false;
  SendTesterPresent(id, 2);
  vTaskDelay( xDelay );
  if ( SetSessionMode(id, mode )) {
    vTaskDelay( xDelay );
    SendTesterPresent(id, 2);
    vTaskDelay( xDelay );
    ESP_LOGV(TAG, "Send Command %03x Service %02x : %02x %02x %02x %02x %02x %02x",
      id, serviceId, b1, b2, b3, b4, b5, b6);
    result = SendCanMessage_sync(id, count, serviceId, b1, b2, b3, b4, b5, b6 );
  }
  SetSessionMode(id, UDS_DEFAULT_SESSION);
  XDISARM;
  return result;
}

/**
 * Send command to IGMP
 * 770 04 2F [b1] [b2] [b3]
 */
bool OvmsHyundaiIoniqEv::Send_IGMP_Command( uint8_t b1, uint8_t b2, uint8_t b3)
{
  XARM("OvmsHyundaiIoniqEv::Send_IGMP_Command");
  bool res = SendCommandInSessionMode(INTEGRATED_GATEWAY, 4, UDS_SID_IOCTRL_BY_ID, b1, b2, b3, 0, 0, 0, UDS_EXTENDED_DIAGNOSTIC_SESSION );
  XDISARM;
  return res;
}

/**
 * Send command to Body Control Module
 * 7a0 04 2F [b1] [b2] [b3]
 */
bool OvmsHyundaiIoniqEv::Send_BCM_Command( uint8_t b1, uint8_t b2, uint8_t b3)
{
  XARM("OvmsHyundaiIoniqEv::Send_BCM_Command");
  bool res = SendCommandInSessionMode(BODY_CONTROL_MODULE, 4, UDS_SID_IOCTRL_BY_ID, b1, b2, b3, 0, 0, 0, UDS_EXTENDED_DIAGNOSTIC_SESSION );
  XDISARM;
  return res;
}

/**
 * Send command to Smart Key Unit
 * 7a5 [b1] 2F [b2] [b3] [b4] [b5] [b6] [b7]
 */
bool OvmsHyundaiIoniqEv::Send_SMK_Command( uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7)
{
  XARM("OvmsHyundaiIoniqEv::Send_SMK_Command");
  bool res = SendCommandInSessionMode(SMART_KEY_UNIT, b1, UDS_SID_IOCTRL_BY_ID, b2, b3, b4, b5, b6, b7, UDS_EXTENDED_DIAGNOSTIC_SESSION);
  XDISARM;
  return res;
}

/**
 * Send command to ABS & EBP Module
 * 7d5 03 30 [b1] [b2]
 */
bool OvmsHyundaiIoniqEv::Send_EBP_Command( uint8_t b1, uint8_t b2, uint8_t mode)
{
  XARM("OvmsHyundaiIoniqEv::Send_EBP_Command");
  bool res = SendCommandInSessionMode(ABS_EBP_UNIT, 3, UDS_SID_IOCTRL_BY_LOC_ID, b1, b2, 0, 0, 0, 0, mode );
  XDISARM;
  return res;
}

/**
 * Send command to Air Conditioning (HVAC) Module
 * 7b3 [len] [service] [b1] [b2] [b3] [b4]
 * Uses VEHICLE_POLL_TYPE_READDATA (0x22) for writing climate settings
 */
bool OvmsHyundaiIoniqEv::Send_AirCon_Command(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4)
{
  XARM("OvmsHyundaiIoniqEv::Send_AirCon_Command");
  // AirCon ECU: 0x7b3 -> 0x7bb
  // Note: Climate commands may require repeated sending ("spamming") to work reliably
  bool res = SendCommandInSessionMode(0x7b3, 4, UDS_SID_IOCTRL_BY_ID, b1, b2, b3, b4, 0, 0, UDS_EXTENDED_DIAGNOSTIC_SESSION);
  XDISARM;
  return res;
}

/**
 * Send climate control command sequence
 * Based on reverse engineering from Discord community
 * Requires intercept harness and canwrite enabled
 * 
 * Command structure for HVAC control:
 * - Start: 0x22 0x01 0x00 [mode] where mode: 0x01=ON, 0x00=OFF
 * - Duration: Optional, in minutes (0 = until manually stopped)
 */
bool OvmsHyundaiIoniqEv::SendClimateCommand(bool start, int duration_minutes)
{
  XARM("OvmsHyundaiIoniqEv::SendClimateCommand");
  
  // Additional safety: require vehicle in Park for climate control
  if (StdMetrics.ms_v_env_gear->AsInt() != 0) {
    ESP_LOGE(TAG, "Climate control blocked: vehicle not in Park (gear=%d)", StdMetrics.ms_v_env_gear->AsInt());
    XDISARM;
    return false;
  }
  
  ESP_LOGI(TAG, "Climate control: %s (duration: %d min)", start ? "START" : "STOP", duration_minutes);
  
  // Send initial command
  uint8_t mode = start ? 0x01 : 0x00;
  bool result = Send_AirCon_Command(mode, 0x00, 0x00, 0x00);
  
  if (result && start) {
    // Send repeated "spam" messages to ensure command is processed
    // This is necessary based on community reverse engineering findings
    SendClimateSpamMessages(true);
    
    m_climate_active = true;
    m_climate_duration_minutes = duration_minutes;
    m_climate_start_time = esp_timer_get_time() / 1000000; // seconds
    
    if (duration_minutes > 0) {
      ESP_LOGI(TAG, "Climate will auto-stop after %d minutes", duration_minutes);
    }
  } else if (result && !start) {
    m_climate_active = false;
    m_climate_duration_minutes = 0;
  }
  
  XDISARM;
  return result;
}

/**
 * Send repeated climate control messages ("spamming")
 * Based on community findings, this increases reliability of climate commands
 * Sends the same command multiple times with short delays
 */
void OvmsHyundaiIoniqEv::SendClimateSpamMessages(bool start)
{
  XARM("OvmsHyundaiIoniqEv::SendClimateSpamMessages");
  
  uint8_t mode = start ? 0x01 : 0x00;
  
  // Send 3-5 repeated commands with 100ms delays
  // This pattern was discovered by the Ioniq5 CAN reverse engineering community
  for (int i = 0; i < 3; i++) {
    Send_AirCon_Command(mode, 0x00, 0x00, 0x00);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  
  XDISARM;
}
#endif
