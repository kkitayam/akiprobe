#include "gtest/gtest.h"
#include "tusb.h"
#include "cmsis_dap_device.h"
#include "mock_tinyusb.h"

using ::testing::DoAll;
using ::testing::SetArgPointee;
using ::testing::Return;

TEST(Class, open)
{
  MockTinyUsb mtu;
  enum { ITF_NUM_VENDOR = 0, ITF_NUM_TOTAL };
  enum { EPNUM_VENDOR_IN = 5, EPNUM_VENDOR_OUT = 5};
  uint8_t rhport = 0;
  uint8_t const itf_desc[] = {
    // Interface number, string index, EP Out & IN address, EP size
    TUD_VENDOR_DESCRIPTOR(ITF_NUM_VENDOR, 5, EPNUM_VENDOR_OUT, 0x80 | EPNUM_VENDOR_IN, TUD_OPT_HIGH_SPEED ? 512 : 64)
  };

  mtu_set_instance(&mtu);
  EXPECT_CALL(mtu, usbd_open_edpt_pair)
    .Times(1)
    .WillOnce(DoAll(SetArgPointee<4>(EPNUM_VENDOR_OUT),
                    SetArgPointee<5>(0x80 | EPNUM_VENDOR_IN),
                    Return(true)));
  EXPECT_CALL(mtu, usbd_edpt_xfer)
    .Times(1)
    .WillOnce(Return(true));

  cmsis_dapd_init();
  cmsis_dapd_reset(rhport);

  ASSERT_NE(0,
            cmsis_dapd_open(rhport, reinterpret_cast<tusb_desc_interface_t const*>(&itf_desc[0]), sizeof(itf_desc))
            );
}
