# Python3
# W1nz4c4r

# ===================Usage=====================
# Python3 node-153.py -CS -R
# Python3 node-153.py -CS -T
# Clean and start ION node only
# Python3 node-153.py -CS
# Configure node as Trasmiter
# Python3 node-153.py -T
# configure node as Receiver
# Python3 node-153.py -R
# =============================================


# imports
import os
import sys
import subprocess
import pyion
import signal

#Global variables
ION_NUM = 0

#handle ctrl+c exit
def def_handler(sig,frame):
    print('\n\n[-] Closing DTN-ION program...')
    sys.exit(1)

# Ctrl + C
signal.signal(signal.SIGINT, def_handler)

#print help panel for the tool
def usage():
    print('[+] Usage: python3 node.py [Options] host##.rc')
    print('-CS, \tClean and Start ION node service.')
    print('-T, \tBP Tramiter.')
    print('-R, \tBP Receiver.')
    sys.exit(1)

#clean & start ION node service
def clean_and_start():
    print("[+] stoping the service\n\n")
    subprocess.run('ionstop')
    text = str(sys.argv[3])
    text.split()
    ion_value = text[5:-3]
    ION_NUM = ion_value
    print('\n\n[+] Starting service on host :{} '.format(ion_value))
    subprocess.run("ionstart -I " + sys.argv[-1], shell=True)

#this set the nodes as receivers
def set_receiver():
    print('\n\n[!] Settng node as Receiver\n\n')
    # Create a proxy to node # and attach to it
    proxy = pyion.get_bp_proxy(ION_NUM)
    proxy.bp_attach()

    # Listen to 'ipn:2.1' for incoming data
    with proxy.bp_open('ipn:4.25') as eid:
        while eid.is_open:
            try:
                # This is a blocking call.
                print('Received:', eid.bp_receive())
            except InterruptedError:
                # User has triggered interruption with Ctrl+C
                break

def set_trasmiter():
    print('\n\n[!] Settng node as Trasmiter\n\n')
    #create a proxy to node # and attach ION
    proxy  = pyion.get_bp_proxy(ION_NUM)
    proxy.bp_attach()

    #openning endpoint and sendin information

    with proxy.bp_open("ipn:4.12") as eid:
        print(' [!] Seding data ... ')
        eid.bp_send('ipn:11.3', b'hello')



def main():
    CS_value = False
    R_value = False
    T_value = False
    Host_value = False


    for i in range(len(sys.argv)):
        if(sys.argv[i] == '-CS' or sys.argv[i] == '-cs'):
            CS_value = True
        elif(sys.argv[i] == '-T' or sys.argv[i] == '-t'):
            T_value = True
        elif(sys.argv[i] == '-R' or sys.argv[i] == '-r'):
            R_value = True

    if (CS_value):
        clean_and_start()

    if (R_value):
        set_receiver()
    if (T_value):
        set_trasmiter()
    if(T_value == False and R_value == False and CS_value == False):
        usage()



if __name__ == '__main__':
    print("DTN - Group 19 - FAU")
    if ( (len(sys.argv)) < 2 ):
        #print help for user
        usage()
    elif( (len(sys.argv)) >= 2 ):
        # if lenght correct send flow to main
        main()
