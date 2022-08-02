#python3
#W1nz4c4r

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

#IMPORTS
import os
import sys
import subprocess
import pyion
import signal
import time

#GLOBAL VARIABLES
PRG_END = True
node_number = 0


# call when closing the program
def def_handler(sig, frame):
    print('\n\n[!] Closing DTN-ION Program...')
    sys.exit(1)

# Ctrl + C --> Close program
signal.signal(signal.SIGINT, def_handler)

#show the instructions
def usage():
    #create usage man
    print('Break')
#if enough parameters
#variables explication
## CS_value --> clean_and_start
# R_value --> set node as receiver
# T_value -->set node as Trasmiter
def main():
    #this variables will make the program flow
    CS_value = False
    R_value = False
    T_value = False
    Exit_value = False
    EPS_value = False #EndPoint Status Value
    ION_NUM = 0
    host_Name = ''

    subprocess.run('clear')
    while PRG_END:
        user_value = input('\nDTN-ION>> ')
        #instructions will be splited and done separatelly
        instruc  = user_value.split()
        for i in range(len(instruc)):
            # send to clean_and_start
            if(instruc[i] == '-CS' or instruc[i] == '-cs'):
                #subprocess.run('clear')
                CS_value = True
            #send to set_Trasmiter
            elif(instruc[i] == '-T' or instruc[i] == '-t'):
                T_value = True
            #send to set_Receiver
            elif(instruc[i] == '-R' or instruc[i] == '-r'):
                R_value = True
            elif(instruc[i] == '-EPS' or instruc[i] == '-eps'):
                EPS_value = True

            #close/leave program
            elif(instruc[i] == '-Q' or instruc[i] == '-q'):
                print('\n\t===== Leaving program!!! =====')
                print('\n[-] Please wait...')
                sys.exit(1)

        #first check if Clean and start is needed
        if(CS_value):
            print('\n\t===== Clean and Start sevice =====')
            ION_NUM = int(input('[!] Please enter the node number you wish to start: '))
            set_node_number(ION_NUM)
            print('[+] Stopping and Starting ION services on specified node : #{}'.format(ION_NUM))
            host_Name = 'host-{}.rc'.format(ION_NUM)
            print('[+] Name  = ' + host_Name)
            clean_and_start(host_Name, ION_NUM)
            CS_value = False
        #second if inputed check endpoint Status
        if(EPS_value):
            print ("\n\t ===== Check Enpoint Status =====")
            check_enpoints()
            EPS_value = False
        if (R_value):
            set_receiver()
            R_value - False
        if (T_value):
            print('\n\t===== Setting node as Trasmiter =====')
            set_Trasmiter(ION_NUM)
            T_value = False

#set node number
def set_node_number(value):
    node_number = value
#get node number
def get_node_number():
    return node_number

# check own endpoint status
def check_enpoints():
    print('The number is: ')
    print(get_node_number())
    #print("[+] Checking the first 100 services on node #{}".format(number))

    #for i in range(0, 100):
    #    ipn_node = 'ipn:' + number + '.'
    #    ipn_node += i
    #    print(ipn_node)


def set_Trasmiter(number):
        #create a proxy to node # and attach ION
        #proxy  = pyion.get_bp_proxy(ION_NUM)
        #proxy.bp_attach()

        #openning endpoint and sendin information
        #with proxy.bp_open("ipn:4.12") as eid:
        #    print(' [!] Seding data ... ')
        #    eid.bp_send('ipn:11.3', b'hello')
        print( 'transmission')

#clean and start ION node service
def clean_and_start(host, number):
    print("[+] Stoping the service\n\n")
    subprocess.run('ionstop')
    print('\n\n[+] Starting service on host :{} '.format(number))
    #start ion service on the node
    subprocess.run("ionstart -I " + host, shell=True)



if __name__ == '__main__' :
    print("DTN - Group 19 - FAU")
    if ( (len(sys.argv)) > 2 ):
        #print help for user
        usage()
    elif( (len(sys.argv)) < 2 ):
        # if lenght correct send flow to main
        main()
