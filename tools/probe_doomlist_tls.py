"""Check the public endpoint's TLS versions with certificate verification enabled."""
import ssl, socket, warnings
warnings.simplefilter('ignore',DeprecationWarning)
for version in (ssl.TLSVersion.TLSv1_1,ssl.TLSVersion.TLSv1_2):
    ctx=ssl.create_default_context()
    ctx.minimum_version=ctx.maximum_version=version
    ctx.set_ciphers('DEFAULT:@SECLEVEL=0')
    try:
        with ctx.wrap_socket(socket.create_connection(('doomlist.net',443),timeout=10),server_hostname='doomlist.net') as sock:
            print(version.name,':',sock.version(),sock.getpeercert()['issuer'])
    except (OSError,ssl.SSLError) as e:
        print(version.name,':',type(e).__name__,str(e))
