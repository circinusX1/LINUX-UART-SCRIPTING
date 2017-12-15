##  Monex Ultra-C and Kiosoft Software Scripted Transaction

### Make a payment transaction in minutes with autocom and ultra-c.kiosft scripts.

```javascript

dofile("include.sss");


/*
    This script:
        - connect to ultra-C device on serial. See include.sss for comport setting.
        - disables power saving.
        - prints device information, Can be commented
        - sends a transaction of 5c to kiosk host server. Make sure you've ran sync.sss, and host ip's are setup properly.
        - waits for completion. The function  saleTransacrionReq in include.sss, prints all transaction id's
        - for success / failure, can open another device as
            - ssh
            - serial
            - a socket
            This example togles DTR and RTS, wires from the serial, I have 2 LED'd in contral parralel on these.
            and creates a file in local folder success, or failed.
            - Can log transaction in log files, and recicle them (disabled in autcom code , (work in proigress for Windows))
*/


function main(zzz)
{
    Debug(0);  // debugs Device level received send buffers
    print("Starting \n");

    local serial = Device(S.SERIAL_PORT, "serial", BINARY);
    enquire(serial, T.Disable_Power_Request, T.Disable_Power_Response, null);
    if(saleTransacrionReq(serial, 5, 120000))
    {
        print ("\n DTR 0 RTS 1  Successful Transaction\n");
        serial.escape("DTR", 0);
        local file = Device("file://success.txt", "file", TEXT);
        file.putsnl("success");
        Sleep(3000);
        file.remove();
    }
    else
    {
        print ("\n DTR 0 RTS 1  Failed Transaction\n");
        serial.escape("DTR", 1);
        serial.escape("RTS", 0);
        local file = Device("file://fail.txt", "file", TEXT);
        file.putsnl("failed");
        Sleep(3000);
        file.remove();
    }
    enquire(serial, T.Sw_Enable_Power_Request, T.Sw_Enable_Power_Response,null);
    return Exit();
}

```


Marius C. Dec 2017





