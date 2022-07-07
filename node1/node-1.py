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


#handle ctrl+c exit
def def_handler(sig,frame):
    print('\n\n[-] Closing...')
    sys.exit(1)

# Ctrl + C
signal.signal(signal.SIGINT, def_handler)

#print help panel for the tool
def usage():
    print('[+] Usage: python3 node-149-py [Options]')
    print('-CS, \tClean and Start ION node service.')
    print('-T, \tBP Tramiter.')
    print('-R, \tBP Receiver.')
    sys.exit(1)

#clean & start ION node service
def clean_and_start():
    print("[+] stoping the service")
    subprocess.run('ionstop')



def main():
    if (sys.argv[1] == '-CS'):
        clean_and_start()
    else:
        usage()

if __name__ == '__main__':
    print("DTN - Group 19 - FAU")
    if ( (len(sys.argv)) < 2 ):
        #print help for user
        usage()
    elif( (len(sys.argv)) >= 2 ):
        # if lenght correct send flow to main
        main()
