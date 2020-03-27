
#### ./autocom scriptfile.ext


```javascript
local PROMPT = "pi@miniban$ ";
local comport = Proxy("tty://dev/ttyUSB0,115200,8N1","proc");
comport.expect("login",-1);
comport.write("pi");
comport.expect("Password");
comport.expect(PROMPT);
comport.write("sudo apt-get update");
comport.expect(PROMPT);

function proc(out)
{   
    if(out.find("Password"))
      comport.write("raspberry");
    print(out);
}

```

#### SDK

Proxy class Members
    c-tor ("connection-string", "outprocedure")
        tty://port,speed,attr
        calls outprocedure for each output received.
    write(string)
    bwrite(array-of-bytes)  writes array of bytes
    ctrl(A..Z)              sends Ctrl+key
    sendkey(ESC,ENTER,HOME,RETURN,UP,DN,LEFT,END,HOME...);
    expect()                waits till something comes out from comm. returns 0
    expect(to)              waits to millisconds for some response. return 0 if expected
    expect(string, to)      expects that string is parto of response as a substring.
    bexpect(array)
    bexpect(array, to)      expects array to be part of returner bynary data
    noop(to)                returns when there is no output activity for at least to mseconds 
    close()
    
Script Object members    
    Script.exit()           When you want to end the script. Must be last line otherwise sript will stay alive.
    Script.sleep(ms)          
    Script.debug(0/1)       turn debugging logs on / off
    Script.printascii(byte-array);
    Script.printbin(byte-array);
    
    
    
    Not for corporations. To be used by sole programmers, In house use only.
    Corporations, Ltd's Inc's SRL's, would need special written license from the author.
    
    
    
    
    
