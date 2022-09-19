#python3
#W1nz4c4r

# ===================Usage=====================
#START PROGRAM
# python3 node.py
#PROGRAM COMMANDS
# -CS = clean and start
# -T = set node as Trasmiter
# -R = set node as Receiver
# -EPS = Check EndPoint Status (Default to 100)
# =============================================

#IMPORTS
import os
import sys
import subprocess
import pyion
import signal
import time
import math
import datetime
import pytz


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
    print('========= instructions goes here ====')
#if enough parameters
#variables explication
## CS_value --> clean_and_start
# R_value --> set node as receiver
# T_value -->set node as Trasmiter


def main():
    #this variables will make the program flow
    CS_value = False # clean and start
    R_value = False # Receiver value
    T_value = False # Trasmiter Value
    Exit_value = False
    EPS_value = False #EndPoint Status Value
    ION_NUM = 0 #host.rc number
    host_Name = ''

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
            #send the program to create_EP_list
            elif(instruc[i] == '-EPS' or instruc[i] == '-eps'):
                EPS_value = True
            #close/leave program
            elif(instruc[i] == '-Q' or instruc[i] == '-q'):
                print('\n\t===== Leaving program!!! =====')
                print('\n[-] Please wait...')
                sys.exit(1)
            else:
                #show instructions
                usage()

        #first check if Clean and start is needed
        if(CS_value):
            print('\n\t===== Clean and Start sevice =====')
            ION_NUM = int(input('[!] Please enter the node number you wish to start: '))
            test_number =  ION_NUM
            print('[+] Stopping and Starting ION services on specified node : #{}'.format(ION_NUM))
            host_Name = 'host-{}.rc'.format(ION_NUM)
            print('[+] Name  = ' + host_Name)
            clean_and_start(host_Name, ION_NUM)
            CS_value = False
        #second if inputed check endpoint Status
        if(EPS_value):
            print ("\n\t ===== Check Enpoint Status =====")
            if ION_NUM == 0 :
                new_number = int(input('[!] Please verify the node number you want to check: '))
                check_enpoints(new_number)
            else:
                check_enpoints(test_number)
            EPS_value = False
        if (R_value):
            print('\n\t===== Setting node as Receiver =====')
            if ION_NUM == 0 :
                ION_NUM = int(input('[!]Please confirm the Node number you wish to set as Receiver: '))
                set_Receiver(ION_NUM)
            else:
                set_Receiver(ION_NUM)
            R_value - False
        if (T_value):
            print('\n\t===== Setting node as Trasmiter =====')
            if ION_NUM == 0 :
                ION_NUM = int(input('[!]Please confirm the Node number you wish to set as Trasmiter: '))
                set_Trasmiter(ION_NUM)
            else:
                set_Trasmiter(ION_NUM)
            T_value = False


#will check if the input by the user is a number o a sstring
#HELPER FUNCTION for create_EP_list
#param:
#   - Valor = set_range from check_enpoints (user input for range creation)
def Check_Input(valor):
    try:
        #try convert to into integer
        value = int(valor)
        return True
    except ValueError:
        #else user input is a sstring
        Return =  False



# check own endpoint status
# params :
## number = ION_number
## rng= range --> total of services that what to check ( default to 101 --> count up to 100)
def check_enpoints(number,rng=101 ):
    EP_list = [] #hold the list form 0-100 on the specified node number
    #this can be eiher a string or nunber (affects what the progran does the range for checking status)
    set_range =input('\n[+] Please enter range to verify OR enter C to continue:')
    print('\n[+]Checking availiable services...')
    path = Check_Input(set_range)
    #path : this variable will let the program where to go
    #   True for number
    #   False for string
    if path:
        set_range = int(set_range) +1
        EP_list= create_EP_list(set_range, number)
    else:
        if (set_range == 'C'  or set_range == 'c'):
            #if user enter C or c it will create the list
            print('[!] Checking first 100 services!')
            EP_list = create_EP_list(rng, number)
        else:
            # if set_range is not a valid input--> call function again
            check_enpoints(number, rng)
    print('\n\n[+] Services up in node #{}: '.format(number))
    send_list = [] #this will send the result to set_get_EPlist
    for i in range(len(EP_list)):
        #True if endopoint is up
        #False if endopoint is off
        EP_Val = pyion.admin.bp_endpoint_exists(EP_list[i])
        if EP_Val:
            send_list.append(i)
            print('[*] {} --> {}\t[UP]'.format(i, EP_list[i]))




#helpter to create the EndPoint Statust list with specified param
def create_EP_list(rng, number):
    list = []
    for i in range(0, rng):
        #creating the list of services from 0-100 to be tested
        ipn_node = 'ipn:' + str(number) + '.'
        ipn_node += str(i)
        list.append(ipn_node)
    return list




# will set the node as trasmiter, will send information a specified node
def set_Trasmiter(number):
    service = ''
    path = 8
    #create a proxy to node # and attach ION
    proxy = pyion.get_bp_proxy(number)
    proxy.bp_attach()


    #openning endpoint and sending information
    service = input('\n[!] Please enter the service number you want to set as trasminter:')
    #path : this variable will let the program where to go
    path = Check_Input(service)
    #   True for number
    #   False for string
    ipn = "ipn:{}.{}".format(number,service)

    if path :
        # Open endpoint 'ipn:1.1' and send data to 'ipn:2.1'
        f = open("file.txt", "r")
        with proxy.bp_open(ipn) as eid:
            for line in f:
                if line.strip() == "": #if the line is empty dont send it.
                    continue
                else:
                    payload = line + "       " +str(datetime.datetime.now(pytz.timezone('US/Eastern')))
                    eid.bp_send('ipn:22.14', payload)

    else:
        print ("This is a String !!!")
        
        
        



#helper  function envharge of activating the user desired service
#param:
#   - node = ipn:#.#
def Activate_trasmiter_service(node):

    discard = input("[+] Please select the mode you wish to set up the service. \n[!] Use D to discard and Q to queue \n[!] To go back enter -BK [*] Mode: ")
    discard_value = Check_Input(discard)
    #   True for number
    #   False for string
    if discard_value:
        #if user enter a number take him back to enter input
        Activate_trasmiter_service(node)
    else:
        #user entered a letter
        if (discard == 'D' or discard == 'd'): #discard Mode
            print ("[!] Node set on Discard mode!")
            pyion.admin.bp_add_endpoint(node, True) #add the new temporary EndPoint
            return
        elif (discard == 'Q' or discard == 'q'): #Queue Mode
            print ("[!] Node set on Queie mode!")
            pyion.admin.bp_add_endpoint(node, False) #add a temporary EndPoint
            return
        elif (dicard == "-BK" or discard == '-bk'):
            print("<--")
            return
        elif( discard == '-Q' or discard == '-q'): #quit the app
            print("[!] Exiting the app ...")
            sys.exit(1)



# will set the node as a receiver and will receive info form the specified node
def set_Receiver(number):
    #recv_number --> is the service number of the receiver 
    recv_number = input('[+] Please enter which service you  wish to use \n\tSERVICE#:  ')
    # Create a proxy to node 2 and attach to it
    proxy = pyion.get_bp_proxy(number)
    proxy.bp_attach()
    #path : this will tell the program where to go
    path = Check_Input(recv_number)
    #True for number
    #False for String
    
    if path:
        #checking specified service
        print("[+] Checking Specified service")
        #Creating the IPN number for the machine 
        ipn = "ipn:{}.{}".format(number, recv_number)
        #checking if node is active 
        node_recv_up = pyion.admin.bp_endpoint_exists(ipn)
        #True if service is up and running
        if node_recv_up:
            print('[+] The service is up and running!!!')


            #set the receiver ready to receive information
            conf_conn = input('[!] Do you wish to set up {} to as a receiver ? (Y/N) '.format(ipn))
            if (conf_conn == 'Y' or conf_conn =='y'):
                # Listen to 'ipn:#.%' for incoming data
                with proxy.bp_open(ipn) as eid:
                    while eid.is_open:
                        try:
                            # This is a blocking call.
                            #NYC time
                            print("=======================START===================== \n")
                            print('[+] DATE ' + str(datetime.datetime.now(pytz.timezone("US/Eastern"))) + " --> Received:", eid.bp_receive())
                            print("=====================END=========================\n")
                        except InterruptedError:
                            # User has triggered interruption with Ctrl+C
                            break
            elif( conf_conn == 'N' or conf_conn =='n'):
                            print("BACK TO TOP")
                            #set_Receiver(number)


        else:
            print("[!]Service is not up")
            

       
    else:
        print("this is a string")




# Helper function to activate a nwe service for the receiver
# params:
#   - node: the node number (ipn:56.2)
def activate_Receiver_service(node):
    mode = input('[+] Please select the MODE you wish to set up the new service. \n[!] Use D to discard and Q to queue \n[!] To go back enter -BK \n\t Mode:')
    mode_value = Check_Input(mode)
    # True for number
    # False for String 
    if mode_value : 
        #if user enters a number take him back to the beginning
        activate_Receiver_service(node)
    else:
        #user entered a letter
        if (mode == 'D' or mode == 'd'): #discard Mode
            print ("[!] Node set on Discard mode!")
            pyion.admin.bp_add_endpoint(node, True) #add the new temporary EndPoint
            return
        elif (mode == 'Q' or mode == 'q'): #Queue Mode
            print ("[!] Node set on Queie mode!")
            pyion.admin.bp_add_endpoint(node, False) #add a temporary EndPoint
            return
        elif (mode == "-BK" or mode == '-bk'):
            print("<--")
            return
        elif( discard == '-Q' or discard == '-q'): #quit the app
            print("[!] Exiting the app ...")
            sys.exit(1)



#clean and start ION node service
def clean_and_start(host, number):
    print("[+] Stoping the service\n\n")
    #stopping ion node
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
        subprocess.run('clear')
        main()
