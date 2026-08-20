#
# mosquitto can also be downloaded from
# https://mosquitto.org/files/binary/win64/mosquitto-2.0.20-install-windows-x64.exe

#
# Install mosquitto
#

net stop mosquitto

# winget install -e EclipseFoundation.Mosquitto -v 2.0.18
. "$PSScriptRoot\helpers.ps1"

$version = "2.0.18"
$url_official = "https://mosquitto.org/files/binary/win64/mosquitto-$version-install-windows-x64.exe"
$url_cache = "https://ci-files01-hki.ci.qt.io/input/windows/mosquitto/mosquitto-$version-install-windows-x64.exe"
$sha1 = "362ececa021a760efb5cf6cab1fa6044a6272fe6"
$package = "C:\Windows\Temp\mosquitto-$version-install-windows-x64.exe"

Download $url_official $url_cache $package
Verify-Checksum $package $sha1

# The Run-Executable is here to make sure the program has been installed
Run-Executable $package "/S"

#
# Configure mosquitto in C:\mosquitto
#

#
# The certificates are generated with
#  openssl req -x509 -sha256 -nodes -newkey rsa:2048 -days 36500 -keyout localhost.key -out localhost.crt
#
# The same certificate need to be used for the tests
#
$cert_key='
-----BEGIN PRIVATE KEY-----
MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQCc1pqVlPX/22kY
vxYZ9+tluPPB8wtvzFHW02RiEFKrdU8Ey8mDBKpK+ljMvzXWC/F+aF1tLs/C7bWx
affMCU5DYq2ATMaMMcFlnj56mytIimmUwbwc1IdL1r5rRcx1FhPdytTbSsi++mY/
xGb9CNAqZFjKG6cxFv86rj4/N1sZItMadq6/xx8p6hb/ChTKCaUdQOrlrZctdldW
kMl2SMjKCFD4Az5rB7TWSgQnM+yLAGB4437xCCfq+JTjrIE143HTFPtIR38jQstf
+sZgcJ3IUqg2X9208hUbuhivs8xSgUsreDAe80Nj9vamMKDSTm7iZD0X6okCSUS5
MMrufUwdAgMBAAECggEAKs2u9q/my2M4NZbBE2lEB0kIzZ/lOSfMFhMvTEwkI8Mq
Q6bSYj19tGTKo2Zz7OzphZQ6GzgxX4O8mKTRChBoGZ/uths9/Lj/jRo49wEuOOf9
lKmjC0M9gYckBObRvArAdUGMAiVQ0D5KdZDGgrxLA6bLTK1rXcxm777qIhqbdCpN
i9RwW9wp9rMV53J8mmWWUDVpfbIKHAmp50JknXj07onPEqN6IKdZY7sYACfYiz/8
l31rKfR/P5j/aRBOFwBnq7Z3JGfOEQPmE8bxQ2Vt3ZXPK2ekreDx4j2Tz+gSZmF3
Ee8DonQtA1DTOlhMyb6FhJrEFmDr38jGQqDK4qV8iQKBgQDLoOSYzX0xcyK2dPMD
woPGcKymI+XYx0Q6cVZo1GttNarhBN0w0ms9a4SYoTMhAI9tzjJw/sU5t9kLan6n
m11PqQVK+Vbh1v2YICJW89LcOFYwyO6IAiMN1LUdy8las4GwcXRwBWWnXAnE+YJK
AiXcmB1ZX3j8pgStCF9VApC50wKBgQDFLP87pGtWW3nvkxoDpgVb1CwpO1T1EsFF
Yzv7eIkFQGUkqF7OlslhMCOERE8pj4dr4CoTIvVsYW+lYfBnFPpQ0pQ+BZ2meZ0u
GfWBDYpNGTGrxEDBc923glb2oegQQirHpiWtlinp5nciOn/rqpUwJR3b46drFlNy
26CTKA+8TwKBgQDKv8qdMo2i/QblMRD+/2CB55KgYkHrVI1ku5DUFB1awgMAxf0P
LZRFtZZy+p6UD6DALn0e8S2jSKE9sq2laRbByINSoW2WtKAQJn7KoT+kshtvu8F1
fts0XERyBITaYL2S14SePWF4ADZiACVwVy8ns/YVFPC8bvlc6Gczl7hOrQKBgQCP
BUmJSIT3GdlDlq7q8JS4fBkgO0IvldAM9aL/j/nLjl8PDPXf4e3mihVpDmdhXRO8
gtfiE5xzZeWmz3iiPMworeYLauVkaJhDZV73ogusStcFoY/bAqFTR76unNgIWwZO
1MxFskNqvtmxQT+igJRAXNvgsKuyeHpFONurggaP7wKBgCj25sAxVcOuOcANQGcO
wCNvt1pHDJengrzaKooCJqm5YxNhrcKlJsdQcQ1Mu+KyhcMY/WF/0UaJZytlHMWf
aDlF7zKqhQFqFY7nk/C7+fED7Kg3XJR54qtweVNzD6k4yTeSsvBeTd2aaepzZvJI
IOQ3RPVQNsdgjn3VTGs/rmrR
-----END PRIVATE KEY-----
'

$cert_crt='
-----BEGIN CERTIFICATE-----
MIID6zCCAtOgAwIBAgIUbbY+pKEPOOytRZ3RujmmdriEAKcwDQYJKoZIhvcNAQEL
BQAwgYMxCzAJBgNVBAYTAk5PMQ0wCwYDVQQIDARPc2xvMQ0wCwYDVQQHDARPc2xv
MQswCQYDVQQKDAJRdDELMAkGA1UECwwCUXQxEjAQBgNVBAMMCWxvY2FsaG9zdDEo
MCYGCSqGSIb3DQEJARYZZXZlbi5vc2Nhci5hbmRlcnNlbkBxdC5pbzAgFw0yNDEw
MzExMTMyNDdaGA8yMTI0MTAwNzExMzI0N1owgYMxCzAJBgNVBAYTAk5PMQ0wCwYD
VQQIDARPc2xvMQ0wCwYDVQQHDARPc2xvMQswCQYDVQQKDAJRdDELMAkGA1UECwwC
UXQxEjAQBgNVBAMMCWxvY2FsaG9zdDEoMCYGCSqGSIb3DQEJARYZZXZlbi5vc2Nh
ci5hbmRlcnNlbkBxdC5pbzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB
AJzWmpWU9f/baRi/Fhn362W488HzC2/MUdbTZGIQUqt1TwTLyYMEqkr6WMy/NdYL
8X5oXW0uz8LttbFp98wJTkNirYBMxowxwWWePnqbK0iKaZTBvBzUh0vWvmtFzHUW
E93K1NtKyL76Zj/EZv0I0CpkWMobpzEW/zquPj83Wxki0xp2rr/HHynqFv8KFMoJ
pR1A6uWtly12V1aQyXZIyMoIUPgDPmsHtNZKBCcz7IsAYHjjfvEIJ+r4lOOsgTXj
cdMU+0hHfyNCy1/6xmBwnchSqDZf3bTyFRu6GK+zzFKBSyt4MB7zQ2P29qYwoNJO
buJkPRfqiQJJRLkwyu59TB0CAwEAAaNTMFEwHQYDVR0OBBYEFGGxHowwihBbJlM5
2wzicQa2sCfwMB8GA1UdIwQYMBaAFGGxHowwihBbJlM52wzicQa2sCfwMA8GA1Ud
EwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBACf4lUaBpw65tOZk32EpMD0D
M0ilx1AwfUqliKdhVvkFJ6jN5h17YhVjFoGtSzE7ry5Rg5J07tUK5Xkjcfk39UWV
RofVnRhrtiticXs0eXQkDXqao0aMjUjsWiokr4/cTNT5R3h9jmjzyjA6pWf2oSjT
GTzZAHdcuBTUfvPy/Ff4M5AVNsI6geUqrUd6XzFwQsDWTT86sdWQIC8j+FMghhss
9XRNOmhWtWGSRbP0HqlWOz5hFRPP/faI/FGTknzkkVYoWaB1CR+ThFK9jYi0FnFL
2fIcJPt+6g6d01Ws5cb9HpL9joUloCjsZ+w6PXYNVyHTaG3Lg5o4nT1Oizvrq1k=
-----END CERTIFICATE-----
'

$mosquitto_conf='
listener 2883
protocol mqtt
allow_anonymous true
max_connections -1

listener 3883
protocol mqtt
allow_anonymous true
max_connections -1

listener 9883
protocol mqtt
allow_anonymous true
max_connections -1
keyfile  C:\Program Files\mosquitto\certs\localhost.key
certfile C:\Program Files\mosquitto\certs\localhost.crt
#tls_version tlsv1

listener 10883
protocol mqtt
allow_anonymous true
max_connections -1
keyfile  C:\Program Files\mosquitto\certs\localhost.key
certfile C:\Program Files\mosquitto\certs\localhost.crt
#tls_version tlsv1

listener 9080
protocol websockets
allow_anonymous true
max_connections -1

listener 10080
protocol websockets
allow_anonymous true
max_connections -1

listener 9081
protocol websockets
allow_anonymous true
max_connections -1
keyfile  C:\Program Files\mosquitto\certs\localhost.key
certfile C:\Program Files\mosquitto\certs\localhost.crt
tls_version tlsv1

listener 10081
protocol websockets
allow_anonymous true
max_connections -1
keyfile  C:\Program Files\mosquitto\certs\localhost.key
certfile C:\Program Files\mosquitto\certs\localhost.crt
tls_version tlsv1
'

New-Item -Path "C:\Program Files\mosquitto" -Name "certs"  -ItemType Directory -ErrorAction SilentlyContinue
New-Item -Path "C:\Program Files\mosquitto" -Name "conf.d" -ItemType Directory -ErrorAction SilentlyContinue

"$cert_key"       | Out-File "C:\Program Files\mosquitto\certs\localhost.key" ascii
"$cert_crt"       | Out-File "C:\Program Files\mosquitto\certs\localhost.crt" ascii
"$mosquitto_conf" | Out-File "C:\Program Files\mosquitto\conf.d\mqtt.conf" ascii

$ss_result = Select-String -Path 'C:\Program Files\mosquitto\mosquitto.conf' -Pattern 'include_dir C:\Program Files\mosquitto\conf.d' -CaseSensitive -SimpleMatch -Quiet

if (-Not $ss_result)
{
  Add-Content -Path 'C:\Program Files\mosquitto\mosquitto.conf' -Value 'include_dir C:\Program Files\mosquitto\conf.d'
}

net start mosquitto
