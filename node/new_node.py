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
        proxy  = pyion.get_bp_proxy(number)
        proxy.bp_attach()

        #openning endpoint and sending information
        service = input('\n[!] Please enter the service number you want to set as trasminter:')
        #path : this variable will let the program where to go
        path = Check_Input(service)
        #   True for number
        #   False for string
        if path:
            print('[+] Checking specified service')
            node = 'ipn:{}.{}'.format(number, service)
            #checking if service is up
            UP_service = pyion.admin.bp_endpoint_exists(node)

            # if true the service is UP
            if UP_service:
                print('[+] Service is up and running!!!')
                #Select the RECEIVER node you wish to connect to
                #bool is made to verify user input, will turn false if input is number
                dst_node_bool = True
                #while input not a number TRUE
                while dst_node_bool:
                    dst_node = input("\n[!] Please enter the Node information to connect to... \n\tNODE #: ")
                    checker = Check_Input(dst_node)
                        #   True for number
                        #   False for string
                    if checker :
                        dst_node_bool = False
                        break
                    else :
                        print("[-] Please enter a the NUMBER of the Node you wish to activate")

                #Select the service you wish to connect to
                #bool is made to verify user input, will turn false if input is number
                dst_service_bool = True
                #while input not a number TRUE
                while dst_service_bool:
                    dst_service = input("\n[!] Please enter the Service number... \n\tSERVICE #: ")
                    checker = Check_Input(dst_service)
                        #   True for number
                        #   False for string
                    if checker :
                        dst_service_bool = False
                        break
                    else :
                        print("[-] Please enter a the SERVICE of the Node you wish to activate")

                receiver_node = "ipn:{}.{}".format(dst_node, dst_service)
                allow = input(("\n[+] You wish to contact {}? (Y or N)".format(receiver_node)))
                if (allow == "Y" or allow =="y"):
                    #user confirmed connection
                    #trasmiting data from specified service
                    with proxy.bp_open(node) as eid:
                        print('\n[!] Seding data ... ')
                        #open file containing data
                        f = open("file.txt", "r")
                        for line in f:
                            if line.strip() == "": #if the line is empty dont send it.
                                continue
                            else:
                                eid.bp_send("ipn:11.17", b'hola1')
                            eid.bp_send("ipn:11.17", "hola2")
                        eid.bp_send("ipn:11.17", "hola3")
                elif(allow == "N" or allow == "n"):
                    #user Denied connection
                    return





            #False = the selected service is down/Not active
            #it will give the option to activate the selected service if desired
            else:
                activate_EP = input('[-] WARNING: service {} its not ACTIVE on node #{}. \nDo you wish to activate it? (Y (yes) 0r N (no))'.format(service, number))
                if (activate_EP == 'Y' or activate_EP == 'y'):
                    print('[+] Activating specified service')
                    #this function is the encharge of creating the temporaty service on the Node
                    Activate_trasmiter_service(node)
                elif (activate_EP == 'N' or activate_EP == 'n'):
                    #If no send user again to main function
                    set_Trasmiter(number)
                elif (activate_EP == '-Q' or activate_EP == '-q'):
                    print("[!] Exiting the app ...")
                    sys.exit(1)
                else:
                    #show usage
                    usage()
                    time.sleep(5)
                    main()

        else:
            if( service == '-EPS' or service == '-eps'):
                print ("\n\t ===== Check Enpoint Status =====")
                check_enpoints(number)
                set_Trasmiter(number)
            elif( service == '-Q' or service == '-q'):
                print("[!] Exiting the app ...")
                sys.exit(1)
            else:
                print ('[*]Please enter a valid service NUMBER \n[*] to check service numbers active use : -EPS \n[*] To exit the app please enter : -Q')
                set_Trasmiter(number)


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

#this is a recursive function that will help the user to specify which receiver node does the user wish to connect # TODO:
def set_Trans_Receiver_node():
    print("show which receiver node")

# will set the node as a receiver and will receive info form the specified node
def set_Receiver(number):
    #create a proxy and attach it to the node
    proxy = pyion.get_bp_proxy(number)
    proxy.bp_attach()
    USER_number = input('please enter the node number you wish to set as a trasmiter ')
    #listen to the data
    with proxy.bp_open('ipn:4.12') as eid:
        while eid.is_open:
            try:
                # This is a blocking call.
                print('\n[+] Received:', eid.bp_receive())
            except InterruptedError:
                # User has triggered interruption with Ctrl+C
                break




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
