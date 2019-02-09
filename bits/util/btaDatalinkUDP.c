

bool BTA_DatalinkInitUDP(void)
{
    return true;
}

bool BTA_ReadyUDP(void)
{
    return false;
}

// todo 1 - move this to a common module. the intent is to allow multiple BTA datalinks....

bool BTA_Ready(void)
{
    return BTA_ReadyUDP() ;
}