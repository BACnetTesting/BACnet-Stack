#include "pch.h"

#include "bitsRouter.h"
#include "bitsUtil.h"

#define MX_CMD_LINE 2

#if defined BAC_DEBUG
extern bool unitTestFail;
#endif

char *cmdLine[MX_CMD_LINE] = {
    "google unit tests",
    "Dolly"
};

class RouterTests : public ::testing::Test
{
protected:

    void SetUp() override
    {
        // Code here will be called immediately after the constructor (right
        // before each test).
        BACnetSetup(MX_CMD_LINE, cmdLine );

#if defined BAC_DEBUG
        if (unitTestFail) {
            FAIL();
            return;
        }
#endif

    }

    void TearDown() override
    {
        // Code here will be called immediately after each test (right
        // before the destructor).
        bits::ShutdownBACnet();
    }
};

extern PORT_SUPPORT *datalinkSupportHead;

TEST_F(RouterTests, Handle_NPDU_Router_Who_Is)
{

    // build a dummy message

    uint8_t rxBuf[MAX_LPDU_IP];
    RX_DETAILS rxDetails;
    ROUTER_MSG rtmsg;

    rxDetails.npdu = rxBuf;
    //rxDetails.npdu_len = pdu_len;
    //rxDetails.portParams = rp->port_support; // more duplication..

    rtmsg.rxDetails = &rxDetails;
    //rtmsg.sourceRouterPort = rp;

    //bacnet_address_clear(&rxDetails.srcPath.glAdr);
    //int bpdu_offset = npci_decode(rxBuf, &rtmsg.dest,
    //    &rxDetails.srcPath.glAdr, &rtmsg.npdu_data);

    //// the shortest NSDU is 1 byte
    //// http://www.bacnetwiki.com/wiki/index.php?title=NSDU
    //if (pdu_len < bpdu_offset + 1) {
    //    // bad incoming packet. todo 0 Log a statistic.
    //    continue;
    //}

    //rtmsg.bpdu_len = pdu_len - bpdu_offset;

    //rtmsg.bpdu = &rxBuf[bpdu_offset];

    // Then submit it

    //handle_npdu_router(&rtmsg);

    EXPECT_EQ(1, 1);
    EXPECT_TRUE(true);
}


// confirm that we see a lon datalink
TEST_F(RouterTests, CreateBFTdatalink)
{
    SetUp();

    for (PORT_SUPPORT *ps = datalinkSupportHead; ps != NULL; ps = (PORT_SUPPORT *)ps->llist.next) {
        if (ps->portType == BPT_FT) {
            SUCCEED();
            TearDown();
            return;
        }
    }

    // not seen, i.e. we fail
    FAIL();
    TearDown();
}


// Confirm that Lon datalink has valid pointer to dual queue handler
TEST_F(RouterTests, BFTdatalinkHasPointer)
{
    SetUp();

    // find Lon datalink
    PORT_SUPPORT *lonDatalink = NULL;

    for (PORT_SUPPORT *ps = datalinkSupportHead; ps != NULL; ps = (PORT_SUPPORT *)ps->llist.next) {
        if (ps->portType == BPT_FT) {
            lonDatalink = ps;
            break;
        }
    }

    GTEST_ASSERT_NE(nullptr, lonDatalink);
    // really just doing this to throw exception if not set to valid memory region
    GTEST_ASSERT_NE(nullptr, lonDatalink->dlSpecific );

    // not seen, i.e. we fail
//    FAIL();
    TearDown();
}

