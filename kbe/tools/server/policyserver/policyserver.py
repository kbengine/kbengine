# -*- coding: utf-8 -*-

import socket

data = """
<?xml version='1.0'?>
<cross-domain-policy>
        <allow-access-from domain="*" to-ports="*" />
</cross-domain-policy>
"""

if __name__ == "__main__":
    s = socket.socket()
    s.bind((b'', 843))
    s.listen(100)

    while(True):
        client, addr = s.accept()
        print("newaddr=%s:%i" % (addr[0], addr[1]))
        #r = client.recv(1024)
        #print("recv=%s" % (r))
        try:
            client.send(data.encode())
        except:
            pass

        try:
            client.close()
        except:
            pass
