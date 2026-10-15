"""Inspect the public TSPG files requested for native download validation."""
from pathlib import Path
import urllib.request
import urllib.error
import ssl
import socket
OUT = Path(__file__).resolve().parents[1] / 'build-3ds'
with urllib.request.urlopen('https://allfearthesentinel.com/zandronum/download.php?file=dwango5.wad',timeout=30) as response:
    data=response.read(10*1024*1024)
    assert data[:4]==b'PWAD' and len(data)<10*1024*1024
    (OUT/'DWANGO5.WAD').write_bytes(data)
    print('DWANGO5.WAD:',len(data),'bytes for target server replay')
for host in ('static.allfearthesentinel.com','www.ifocserv.net','wads.pizza-doom.it'):
    with socket.create_connection((host,443),timeout=20) as connection:
        with ssl.create_default_context().wrap_socket(connection,server_hostname=host) as tls:
            print(host,'issuer:',tls.getpeercert()['issuer'])
            for n,cert in enumerate(tls._sslobj.get_verified_chain()):
                pem=cert.public_bytes()
                (OUT/f'{host}-chain-{n}.der').write_bytes(ssl.PEM_cert_to_DER_cert(pem))
            print('served chain:',[cert.get_info()['subject'] for cert in tls._sslobj.get_unverified_chain()])
for name in ('tspg_connectioncolumn_r1.wad', 'skulltag_content-3.2-beta2.pk3'):
    for root in ('https://static.allfearthesentinel.com/wads/', 'https://allfearthesentinel.com/zandronum/download.php?file=', 'https://www.ifocserv.net/ftp/wads/', 'https://wads.pizza-doom.it/'):
        url = root + name
        request = urllib.request.Request(url, headers={'User-Agent': 'Zandronum3DS/0.1.14'})
        try:
            with urllib.request.urlopen(request, timeout=30) as response:
                print(url, response.status, response.headers.get('Content-Length'), response.headers.get('Content-Type'), response.read(4))
        except urllib.error.HTTPError as error:
            print(url, error.code, error.read(600).decode(errors='replace'))
