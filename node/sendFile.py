import pyion

number = 24

proxy = pyion.get_bp_proxy(number)
proxy.bp_attach()
count = 0 
ipn = "ipn:24.1"
ipn_to_send = "ipn:22.2"
ipn5 = 5
#file names variables 
uv_file = "sensorUV.txt"
rain_file = "rainDrop.txt"
htp_file = "HTP.txt"

liner = []
destination = []
final_numbers = []
new_dst = [] 
with open("host-{}.rc".format(number), "r") as fp:
    line = fp.readlines()
    for row in line:
        word = "a plan" 
        #print(row.find(word))
        if row.find(word) != -1:
            liner.append(row)
for i in range(len(liner)):
    tester = liner[i].split('/') 
    destination.append(tester[1])
for x in range(len(destination)):
    host_file_numbers = destination[x].split('\n')
    final_numbers.append(int(host_file_numbers[0]))

for y in range(len(final_numbers)):
    if final_numbers[y] != number:
        new_dst.append(final_numbers[y])


inHost = False
for i in range(len(new_dst)):
    if  new_dst[i] == ipn5:
        inHost = True
        continue
    else:
        inHost =  False

if inHost:
    print("dest is in host file")
else:
    print("dest not in host")

















    #starting seding the information
#openning UV sensor file information
try:
    for x in range(4):
    if x == 0:
    #send the IPN ion information
    print('Seding the IPN')
    with proxy.bp_open(ipn) as eid:
        eid.bp_send(ipn_connection, ipn_connection)
    elif x == 1 :
    print('Sending file UV file')
    with proxy.bp_open(ipn) as eid:
        eid.bp_send_file(ipn_connection, uv_file )
    elif x == 2 :
    print('Sending file rain file')
    with proxy.bp_open(ipn) as eid:
        eid.bp_send_file(ipn_connection, rain_file)
    elif x == 3 :
    print('Sending file HTP file')
    with proxy.bp_open(ipn) as eid:
        eid.bp_send_file(ipn_connection, htp_file)
except : 
print(Style.BRIGHT + Fore.RED + "[!] Fatal ERROR. Code 652")
    
    