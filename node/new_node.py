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

# Sensors used on this project:
    ## LM393 - Rain Drop Sensor (DONE)
    ## BMP380 - Humidity/Temp/Pressure Sensor (DONE)
    ## VEML6075 - UV Sensor (DONE)
    ## CJMCU-6814 - Particulate Sensor**

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
from colorama import Fore, Back, Style, init 
from pymongo import MongoClient
from halo import Halo


#=================== DATABASE realted information ======================
#Declare main files names
#this is to make the change of the file easier
mongoIP = "0.tcp.ngrok.io:18209"
##BASE FILE NAMES THAT WILL CONTAINT THE ACTUAL DATA --> FINAL RECEIVER  WILL UPLOAD THEM 
#once the data is received each file will have this names
# this names are used in Clean_data() and updateDB()
UV_name = "sensorUV.txt"
rain_name = "rainDrop.txt"
name_HTP = "HTP.txt"

#this will be the files to clean
#this are the original files produced, The ones that are going to be send
UV_name_toClean = "UV Sensor.txt"
rain_name_toClean = "Rain Drop Sensor.txt"
name_HTP_toClean = "BME280.txt"
#=======================================================================


#initializing colorama so it resets every time it prints something
init(autoreset=True)

#GLOBAL VARIABLES
PRG_END = True
node_number = 0

# call when closing the program
def def_handler(sig, frame):
    print(Style.BRIGHT + Fore.CYAN + '\n\n[!] Closing DTN-ION Program...')
    sys.exit(1)

# Ctrl + C --> Close program
signal.signal(signal.SIGINT, def_handler)

#show the instructions
def usage():
    #create usage man
    print(Style.BRIGHT + Fore.YELLOW + '========= how to use =========' ) 
    #Select an option
    print(Style.BRIGHT + Fore.YELLOW + "\n -CS --> Clean and start: this will stop all the ION-DTN existing processes running on the host file")
    print(Style.BRIGHT + Fore.YELLOW + "\n -R --> Set as Receiver")
    print(Style.BRIGHT + Fore.YELLOW + "\n -T --> Set as Transmitter")
    print(Style.BRIGHT + Fore.YELLOW + "\n -Q --> Quit or exit the program")
    print(Style.BRIGHT + Fore.YELLOW + "\n -EPS --> End Point Status: this will let you check which services ar ACTIVE on the node ")
    print(Style.BRIGHT + Fore.YELLOW + "\n -STP --> Stop ION-DTN protocol ")
    print(Style.BRIGHT + Fore.YELLOW + "\n -UDB --> Update Data Base ")
    


def main():
    #this variables will make the program flow
    CS_value = False # clean and start
    R_value = False # Receiver value
    T_value = False # Trasmiter Value
    Exit_value = False
    EPS_value = False #EndPoint Status Value
    STP_value = False #STOP ion protocol 
    UDB_value = False #Update DataBase 
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
            # send the program to stop_ION
            elif(instruc[i] == '-UDB' or instruc[i] == '-udb'):
                UDB_value = True
            # send the program to stop_ION
            elif(instruc[i] == '-STP' or instruc[i] == '-stp'):
                STP_value = True
            #close/leave program
            elif(instruc[i] == '-Q' or instruc[i] == '-q'):
                print(Style.BRIGHT + Fore.CYAN+ '\n\t===== Leaving program!!! =====' ) 
                print(Style.BRIGHT + Fore.CYAN + '\n[-] Please wait...' )
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
            print (Style.BRIGHT + Fore.MAGENTA + "\n\t ===== Check Enpoint Status =====")
            if ION_NUM == 0 :
                new_number = int(input(Style.BRIGHT + Fore.MAGENTA + '[!] Please verify the node number you want to check: '))
                check_enpoints(new_number)
            else:
                check_enpoints(ION_NUM)
            EPS_value = False
        if (R_value):
            print( Style.BRIGHT + Fore.GREEN + '\n\t===== Setting node as Receiver =====')
            if ION_NUM == 0 :
                ION_NUM = int(input(Style.BRIGHT + Fore.GREEN + '[!]Please confirm the Node number you wish to set as Receiver: '))
                set_Receiver(ION_NUM)
            else:
                set_Receiver(ION_NUM)
            R_value - False
        if (T_value):
            print(Style.BRIGHT + Fore.BLUE + '\n\t===== Setting node as Trasmiter =====')
            if ION_NUM == 0 :
                ION_NUM = int(input(Style.BRIGHT + Fore.BLUE + '[!]Please confirm the Node number you wish to set as Trasmiter: '))
                set_Trasmiter(ION_NUM)
            else:
                set_Trasmiter(ION_NUM)
            T_value = False
        if (STP_value):
            print( '\n\t===== Stop ION-DTN Services =====')
            subprocess.run('ionstop')
            STP_value = False
        if(UDB_value):
            print(Style.BRIGHT + Fore.WHITE + '\n\t===== Update Database =====')
            update_information()
            UDB_value = False


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
    set_range =input(Style.BRIGHT + Fore.MAGENTA + '\n[+] Please enter range to verify OR enter C to continue:')
    print(Style.BRIGHT + Fore.MAGENTA + '\n[+]Checking availiable services...')
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
            print(Style.BRIGHT + Fore.MAGENTA + '[!] Checking first 100 services!')
            EP_list = create_EP_list(rng, number)
        else:
            # if set_range is not a valid input--> call function again
            check_enpoints(number, rng)
    print(Style.BRIGHT + Fore.MAGENTA + '\n\n[+] Services up in node #{}: '.format(number))
    send_list = [] #this will send the result to set_get_EPlist
    for i in range(len(EP_list)):
        #True if endopoint is up
        #False if endopoint is off
        EP_Val = pyion.admin.bp_endpoint_exists(EP_list[i])
        if EP_Val:
            send_list.append(i)
            print(Style.BRIGHT + Fore.MAGENTA + '[*] {} --> {}\t[UP]'.format(i, EP_list[i]))


##update information 
#no params needed
# this option is intended for the FINAL RECEIVER (THE ON IN CHARGE OF
# UPLOADING THE DATA TO THE DB TO FUTHER USE ON THE WEB SERVER)
# first will clean the data that was previously received
# then it will upload each record
def update_information():
    sensor_data = Clean_data()
    uploadDB(sensor_data)


# Clean_data is in charge of reading the received files and parse
# the important information
def Clean_data():
    #array that will hold the data that is going to by uploaded 
    uv_Final_Data = []
    rain_Final_Data =[]
    htp_Final_Data = [] 
    #total data will be an array of arrays
    # total_data = [
    #       [uv_Final_Data],
    #       [Rain_Final_Data],
    #       [HTP_final_Data],
    #       [Part_final_Data]
    #   ]
    total_data = []

    #This variables will be added to htp_Final_data
    #htp_Final_Data = [
    #       [Humidity_Data],
    #       [Temp_Data],
    #       [Preasure_Data],
    #       [altitude_Data]
    #   ]
    Humidity_Data = [] 
    Temp_Data = [] 
    Preasure_Data = []
    altitude_Data = [] 
    
    
    #reading the UV_sensor.txt document
    try:
        uv_file = open(UV_name, "r")
        print(Style.BRIGHT + Fore.WHITE + "[!] Cleaning data from the UV file")
        count = 0 
        for line in uv_file:
            if count < 2 : 
                count = count + 1 
                continue
            else:
                #getting only the number information on the file
                uv_parse_data = line.split()
                #adding last number read to the list 
                uv_Final_Data.append(uv_parse_data[3])
        #print(uv_Final_Data)
        #adding uv data to the total data array
        total_data.append(uv_Final_Data)
    except:
        print(Style.BRIGHT + Fore.WHITE + "unable to find {}".format(UV_name))

    #reading the rain sensor data
    try:
        file_rain= open(rain_name, "r")
        print(Style.BRIGHT + Fore.WHITE + "[!] Cleanning data form the rain file")
        #if the file exist then parse the document
        count = 0 
        for line in file_rain:
            if count == 0 or count == 1:
                #this is to ignore the first line of the file
                count = count + 1
                continue
            else:
                rain_parse_data = line.split()
                #array of arrays to be appended to total data
                compact_rain_data=[]
                if len(rain_parse_data) == 3:
                    compact_rain_data = [rain_parse_data[0], rain_parse_data[2]]
                    
                if len(rain_parse_data) == 4:
                    compact_rain_data = [rain_parse_data[0] , "No Rain"]
                #array that will be added to another array
                rain_Final_Data.append(compact_rain_data) 
        #adding the rain data to the total data array   
        #print(rain_Final_Data)
        total_data.append(rain_Final_Data)
        
    except:
        print(Style.BRIGHT + Fore.WHITE + "unable to find {}".format(rain_name))

    #Reading HTP (Humidity-Temperature-Preasure) file 
    try:
        file_HTP = open(name_HTP, "r")
        print(Style.BRIGHT + Fore.WHITE + "[!] Cleanning data form the HTP (Humidity-Temperature-Preasure) file")
        #if the document exist then parse the docuiment
        for line in file_HTP:
            if len(line) >1 :
                line_parsed = line.split()
                # if temperature
                if line_parsed[0] == "Temperature":
                    Temp_Data.append(line_parsed[2])
                elif line_parsed[0] == "Pressure":
                    Preasure_Data.append(line_parsed[2])
                elif line_parsed[0] == "Approx.":
                    altitude_Data.append(line_parsed[3])
                elif line_parsed[0] == "Humidity":
                    Humidity_Data.append(line_parsed[2])

        # adding the HTP data to the total data array
        #adding temp
        total_data.append(Temp_Data)
        #adding preasure
        total_data.append(Preasure_Data)
        #adding humidity
        total_data.append(Humidity_Data)
        #adding altitude
        total_data.append(altitude_Data)
    except:
        print(Style.BRIGHT + Fore.WHITE + "unable to find {}".format(name_HTP))

    print(Style.BRIGHT + Fore.WHITE + "[*] Data has been cleanned!!!")
    return total_data


#this will send all the data gathered to the DB 
def uploadDB(sensor_data):
    print(Style.BRIGHT + Fore.WHITE + "[!] Attempting to connect to Mongo Client!")
    try: 
        conn = MongoClient(mongoIP)
        print(Style.BRIGHT + Fore.WHITE + "[+] Connected to client!")
        #connect to DTN database
        db =  conn.DTN
        for i in range(len(sensor_data)):
            if i == 0 : 
                # 0 --> UV Sensor information
                spinner = Halo(text= Style.BRIGHT + Fore.WHITE + "[!] Uploading UV sensor data", spinner = "dots",  placement = "right")
                spinner.start()
                #upload Data to DB (DTN) to collection (UVsensor)
                #selecting the corresponding collection
                UV_Collection = db.UVsensor
                
                for x in range(len(sensor_data[i])):
                    #parse the UV part to upload data
                    timer = str(datetime.datetime.now(pytz.timezone("US/Eastern")))
                    timer = timer.split()
                    #posting the information
                    #print(sensor_data[i][x])
                    #print("Sensor data is ..... {}".format(sensor_data[i][x]))
                    post = {"Time": timer[1], "Date" : timer[0], "UV Sensor Data": sensor_data[i][x]}
                    #posting information on the DB
                    UV_Collection.insert_one(post)
                
                spinner.succeed()
                spinner.stop()
            elif i == 1: 
                # 1 --> Rain sensor information
                spinner = Halo(text= Style.BRIGHT + Fore.WHITE + "[!] Uploading Rain sensor data", spinner = "dots", placement = "right")
                spinner.start()
                #upload data to DB (DTN) to the collection(RainSensor)
                rain_Collection = db.RainSensor
                for y in range(len(sensor_data[i])):
                    #traversing the rain data
                    #print(sensor_data[i][y])
                    timer = str(datetime.datetime.now(pytz.timezone("US/Eastern")))
                    timer = timer.split()
                    #creating the post 
                    post={"Time": timer[1], "Date" : timer[0], "Rain code": sensor_data[i][y][0], "state" : sensor_data[i][y][1]}
                    #posting the information on the DB
                    rain_Collection.insert_one(post)
                spinner.succeed()
                spinner.stop()
            elif i == 2:
                # 2 --> Temperature data
                spinner = Halo(text= Style.BRIGHT + Fore.WHITE + "[!] Uploading Temperature sensor data", spinner = "dots", placement = "right")
                spinner.start()
                #upload data to DB (DTN) and to the collection (TemperatureSensor)
                temp_Collection = db.TemperatureSensor
                for u in range(len(sensor_data[i])):
                    #Trasversinf data for temperature
                    #print(sensor_data[i][u])
                    timer = str(datetime.datetime.now(pytz.timezone("US/Eastern")))
                    timer = timer.split()
                    #creating the post
                    post = {"Time": timer[1], "Date" : timer[0], "Temperute (C°)": sensor_data[i][u]}
                    #Posting the information in the DB
                    temp_Collection.insert_one(post)
                spinner.succeed()
                spinner.stop()
            elif i == 3:
                # 3 --> Preasure Sensor information
                spinner = Halo(text= Style.BRIGHT + Fore.WHITE + "[!] Uploading Preasure sensor data" ,spinner = "dots", placement = "right")
                spinner.start()
                #upload data to DB (DTN) and to the collection (PressureSensor)
                Preasure_Collection = db.PressureSensor
                for x in range(len(sensor_data[i])):
                    #print(sensor_data[i][x])
                    #trasversing data for preasure 
                    timer = str(datetime.datetime.now(pytz.timezone("US/Eastern")))
                    timer = timer.split()
                    #creating post
                    post = {"Time": timer[1], "Date" : timer[0], "Preasure":  sensor_data[i][x]}
                    #post data to the DB
                    Preasure_Collection.insert_one(post)
                spinner.succeed()
                spinner.stop()
            elif i == 4 :
                #4 --> Humidity sensor information 
                spinner = Halo(text= Style.BRIGHT + Fore.WHITE + "[!] Uploading Humidity sensor data",  spinner = "dots", placement = "right")
                spinner.start()
                #upload data to DB (DTN) and to the collection (HumiditySensor)
                humidity_Collection = db.HumiditySensor
                for x in range(len(sensor_data[i])):
                    #trasversing humidity data
                    timer = str(datetime.datetime.now(pytz.timezone("US/Eastern")))
                    timer = timer.split()
                    #creating post
                    post = {"Time": timer[1], "Date" : timer[0], "Humidity": sensor_data[i][x] }
                    humidity_Collection.insert_one(post)
                spinner.succeed()
                spinner.stop()
            elif i == 5:
                # 5 --> Altitude Sensor information
                spinner = Halo(text= Style.BRIGHT + Fore.WHITE + "[!] Uploading Altitude sensor data",  spinner = "dots", placement = "right")
                spinner.start()
                #upload data to DB (DTN) and to the collection (AltitudeSensor)
                altitude_Collection = db.AltitudeSensor
                for x in range(len(sensor_data[i])):
                    #trasversing altitude data
                    timer = str(datetime.datetime.now(pytz.timezone("US/Eastern")))
                    timer = timer.split()
                    #creating post
                    post = {"Time": timer[1], "Date" : timer[0], "Altitude":  sensor_data[i][x]} 
                    altitude_Collection.insert_one(post)
                spinner.succeed()
                spinner.stop()
            else: 
                spinner = Halo(text= Style.BRIGHT + Fore.RED +"[-] Data provided might be corrumpted",  spinner = "dots", placement = "right")
                spinner.start()
                spinner.fail()
                spinner.stop()            
            
    except:
        print(Style.BRIGHT + Fore.WHITE + "Could not connect!")



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
    service = input(Style.BRIGHT + Fore.BLUE + '\n[!] Please enter the service number you want to set as trasminter: ')
    #path : this variable will let the program where to go
    path = Check_Input(service)
    #   True for number
    #   False for string
    ipn = "ipn:{}.{}".format(number,service)

    

    if path :
        #checking if specified node and service is active 
        #t_node_up --> true if the local node is up 
        t_node_up = pyion.admin.bp_endpoint_exists(ipn)
        if t_node_up:
            print(Style.BRIGHT + Fore.BLUE + "[*] Service is up and running!!!")
            #Getting the destination node and service 
            dest_node = input(Style.BRIGHT + Fore.BLUE + "\n[!] Please enter the NODE number you wish to contact: \n\tNODE #: ")
            #dst_node_check is will tell us if the input given by the user is a number o a string
            dst_node_check = Check_Input(dest_node)
            #   True for number
            #   False for string
            if dst_node_check:
                #the user have enter a number as a node 
                print(Style.BRIGHT + Fore.BLUE + "[*] Node number setted up " )
                dest_service = input(Style.BRIGHT + Fore.BLUE + "\n[!] Please enter the SERVICE you wish to contact: \n\t SERVICE #: ")
                dst_service_check = Check_Input(dest_service)
                #   True for number
                #   False for string
                if dst_service_check:
                    #user enter a number as service
                    print(Style.BRIGHT + Fore.BLUE + "[*] Service number has been setted up ")
                    #finnish confirming the information for setting up the node information to transtmit 
                    ipn_connection = "ipn:{}.{}" .format(dest_node, dest_service) # this is the node that is going to receive the information 
                    print(Style.BRIGHT + Fore.BLUE +  ipn_connection)
                    conf_conn = input(Style.BRIGHT + Fore.BLUE + '[!] Do you wish to set up {} to as a trasmiter ? (Y/N) '.format(ipn))
                    if (conf_conn == 'Y' or conf_conn =='y'):

                        #starting seding the information
                        #openning UV sensor file information
                        try:
                            f = open(UV_name, "r")
                            with proxy.bp_open(ipn) as eid:
                                for line in f:
                                    if line.strip() == "": #if the line is empty dont send it.
                                        continue
                                    else:
                                        #this will be the send time
                                        payload = line + "       " +str(datetime.datetime.now(pytz.timezone('US/Eastern')))
                                        eid.bp_send(ipn_connection, payload)
                        except : 
                            print(Style.BRIGHT + Fore.RED + "[!] Could not open {}".format(UV_name_toClean))

                        
                    elif (conf_conn == 'N' or conf_conn =='n'):
                        set_Trasmiter(number)

                else:
                    print(Style.BRIGHT + Fore.RED + "You entered an incorrect service \nPlease repeat the process and check your input")
                    print(Style.BRIGHT + Fore.BLUE + '\n\t===== Setting node as Receiver =====')
                    set_Trasmiter(number)

            else:
                print(Style.BRIGHT + Fore.RED + "You entered an invalid node caracter - string \nPlease repeat the process and check your input")
                print(Style.BRIGHT + Fore.BLUE + '\n\t===== Setting node as Receiver =====')
                set_Trasmiter(number)
        else:
            print(Style.BRIGHT + Fore.RED + "[!] The service is not up!")
            create_receiver =input(Style.BRIGHT + Fore.RED + "[+] do you wish to activate {}? (Y/N) ".format(ipn))
            #yes will create/activate the new service and that specified node
            #no will go bacl th set_trasmiter
            if (create_receiver == 'y' or create_receiver =='Y'):
                print(Style.BRIGHT + Fore.BLUE + '[*] Activating the Specified Service.')
                #calling the function that will activate the specified service
                Activate_trasmiter_service(ipn)
            elif (create_receiver == 'N' or create_receiver == 'n'):
                #If no send user again to main function
                set_Trasmiter(number)
            elif (create_receiver == '-Q' or create_receiver == '-q'):
                print(Style.BRIGHT + Fore.BLUE + "[!] Exiting the app ...")
                sys.exit(1)
            else:
                #show usage
                usage()
                time.sleep(5)
                main()
    #user entered a wrong option 
    else:
        if( t_node_up == '-EPS' or t_node_up == '-eps'):
            print (Style.BRIGHT + Fore.BLUE + "\n\t ===== Check Enpoint Status =====")
            check_enpoints(number)
            set_trasmiter(number)
        elif( t_node_up == '-Q' or t_node_up == '-q'):
            print(Style.BRIGHT + Fore.BLUE + "[!] Exiting the app ...")
            sys.exit(1)
        else:
            print (Style.BRIGHT + Fore.BLUE + '[*]Please enter a valid service NUMBER \n[*] to check service numbers active use : -EPS \n[*] To exit the app please enter : -Q')
            set_Trasmiter(number)

#helper  function envharge of activating the user desired service
#param:
#   - node = ipn:#.#
def Activate_trasmiter_service(node):

    discard = input(Style.BRIGHT + Fore.BLUE + "[+] Please select the mode you wish to set up the service. \n[!] Use D to discard and Q to queue \n[!] To go back enter -BK [*] Mode: ")
    discard_value = Check_Input(discard)
    #   True for number
    #   False for string
    if discard_value:
        #if user enter a number take him back to enter input
        Activate_trasmiter_service(node)
    else:
        #user entered a letter
        if (discard == 'D' or discard == 'd'): #discard Mode
            print (Style.BRIGHT + Fore.BLUE + "[!] Node set on Discard mode!")
            pyion.admin.bp_add_endpoint(node, True) #add the new temporary EndPoint
            return
        elif (discard == 'Q' or discard == 'q'): #Queue Mode
            print (Style.BRIGHT + Fore.BLUE +  "[!] Node set on Queie mode!")
            pyion.admin.bp_add_endpoint(node, False) #add a temporary EndPoint
            return
        elif (discard == "-BK" or discard == '-bk'):
            print("<--")
            return
        elif( discard == '-Q' or discard == '-q'): #quit the app
            print(Style.BRIGHT + Fore.CYAN + "[!] Exiting the app ...")
            sys.exit(1)



# will set the node as a receiver and will receive info form the specified node
def set_Receiver(number):
    #recv_number --> is the service number of the receiver 
    recv_number = input(Style.BRIGHT + Fore.GREEN +'[+] Please enter which service you  wish to use \n\tSERVICE#:  ')
    # Create a proxy to node 2 and attach to it
    proxy = pyion.get_bp_proxy(number)
    proxy.bp_attach()
    #path : this will tell the program where to go
    path = Check_Input(recv_number)
    #True for number
    #False for String
    
    if path:
        #checking specified service
        print(Style.BRIGHT + Fore.GREEN +"[+] Checking Specified service")
        #Creating the IPN number for the machine 
        ipn = "ipn:{}.{}".format(number, recv_number)
        #checking if node is active 
        node_recv_up = pyion.admin.bp_endpoint_exists(ipn)
        #True if service is up and running
        if node_recv_up:
            print(Style.BRIGHT + Fore.GREEN +'[+] The service is up and running!!!')


            #set the receiver ready to receive information
            conf_conn = input(Style.BRIGHT + Fore.GREEN +'[!] Do you wish to set up {} to as a receiver ? (Y/N) '.format(ipn))
            if (conf_conn == 'Y' or conf_conn =='y'):
                # Listen to 'ipn:#.%' for incoming data
                with proxy.bp_open(ipn) as eid:
                    #opening the file in which data is going to be record
                    write_UV_file = open(UV_name,"w")
                    while eid.is_open:
                        try:
                            # This is a blocking call.
                            #NYC time
                            #print(Style.BRIGHT + Fore.GREEN +"=======================START===================== \n")
                            #print(Style.BRIGHT + Fore.GREEN +'[+] DATE ' + str(datetime.datetime.now(pytz.timezone("US/Eastern"))) + " --> Received:", eid.bp_receive())
                            #print(Style.BRIGHT + Fore.GREEN +"=====================END=========================\n")

                            #receiving the file and writing into the respective place
                            write_UV_file.write(str(datetime.datetime.now(pytz.timezone("US/Eastern"))) + str(eid.bp_receive()))

                        except InterruptedError:
                            # User has triggered interruption with Ctrl+C
                            break
            elif( conf_conn == 'N' or conf_conn =='n'):
                set_Receiver(number)


        else:
            print(Style.BRIGHT + Fore.GREEN +"[!] The service is not up!")
            create_receiver = input(Style.BRIGHT + Fore.GREEN +'[+] do you wish to activate {}? (Y/N)'.format(ipn))
            #yes will create/activate a new service
            #no will go back to set_Receiver
            if (create_receiver == 'y' or create_receiver =='Y'):
                print(Style.BRIGHT + Fore.GREEN +'[*] Activating the Specified Service.')
                #calling the function that will activate the specified service
                activate_Receiver_service(ipn)
            elif (create_receiver == 'N' or create_receiver == 'n'):
                #If no send user again to main function
                set_Trasmiter(number)
            elif (create_receiver == '-Q' or create_receiver == '-q'):
                print(Style.BRIGHT + Fore.CYAN +"[!] Exiting the app ...")
                sys.exit(1)
            else:
                #show usage
                usage()
                time.sleep(5)
                main()

    #user entered a wrong option
    else:
        if( recv_number == '-EPS' or recv_number == '-eps'):
                print (Style.BRIGHT + Fore.MANGENTA +"\n\t ===== Check Enpoint Status =====")
                check_enpoints(number)
                set_Receiver(number)
        elif( recv_number == '-Q' or recv_number == '-q'):
            print(Style.BRIGHT + Fore.CYAN +"[!] Exiting the app ...")
            sys.exit(1)
        else:
            print (Style.BRIGHT + Fore.GREEN +'[*]Please enter a valid service NUMBER \n[*] to check service numbers active use : -EPS \n[*] To exit the app please enter : -Q')
            set_Receiver(number)

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
