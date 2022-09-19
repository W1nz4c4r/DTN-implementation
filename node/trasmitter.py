import os
import subprocess
from datetime import datetime
from threading import Thread
import time

# Import module
import pyion
from pyion import BpCustodyEnum, BpPriorityEnum, BpReportsEnum

# =================================================================
# === Define global variables
# =================================================================

# ION node number
node_nbr = 1

# Originating and destination endpoints
orig_eid = 'ipn:24.1'
dest_eid = 'ipn:22.1'
rept_eid = 'ipn:24.2'

# Define endpoint properties
ept_props = {
    'TTL':          3600,   # [sec]
    'priority':     BpPriorityEnum.BP_EXPEDITED_PRIORITY,
    'report_eid':   rept_eid,
    'report_flags': BpReportsEnum.BP_RECEIVED_RPT
    #'report_flags': BpReportsEnum.BP_RECEIVED_RPT | BpReportsEnum.BP_CUSTODY_RPT,
}

# Create a proxy to ION
proxy = pyion.get_bp_proxy(node_nbr)

# Attach to ION
proxy.bp_attach()

# =================================================================
# === Acquire reports
# =================================================================

# Open endpoint to get reports
rpt_eid = proxy.bp_open(rept_eid)

def print_reports():
    while True:
        try:
            data = rpt_eid.bp_receive()
            print(data)
        except InterruptedError:
            break

# Start monitoring thread
th = Thread(target=print_reports, daemon=True)
th.start()

# =================================================================
# === MAIN
# =================================================================

# Open a endpoint and set its properties. Then send file
with proxy.bp_open(orig_eid, **ept_props) as eid:
    for i in range(50):
        eid.bp_send(dest_eid, str(datetime.now()) + ' - ' + 'a'*1000)

# Sleep for a while and stop the monitoring thread
time.sleep(2)
#proxy.bp_interrupt(rept_eid)
th.join()
