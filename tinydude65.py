#   tinydude65.py
#
#   host program for the tinyload65 bootloader
#
#   features:
#       - upload binary files to the target
#       - download binary files from the target
#       - test tinyload65 nmi handling
#
#   remarks:
#       - minimal implementation of the tinyload65 protocol
#       - allows testing of bootloader functions
#       - some code cleanup and refactoring would be nice
#       - dtr/rts handling based on trial and error, may need adjustment
#------------------------------------------------------------------------------

import sys
import os
import argparse
import serial

#------------------------------------------------------------------------------
verbose = 0

def die(msg):
    print(msg)
    exit(1)

def log(msg):
    if verbose > 0:
        print(msg)

def dbg(msg):
    if verbose > 1:
        print(msg)

#------------------------------------------------------------------------------
#   receive a packet from the target, check size and command

def rx_packet(ser, min_size = 1, cmd = None):
    data = ser.read(1)
    if len(data) == 0:
        die("packet timeout")

    size = data[0]
    if size == 0:
        size = 256
    if size < min_size:
        die("invalid packet size %d" % size)
    dbg("rx_packet size %d" % size)

    data = ser.read(size - 1)
    if len(data) != size - 1:
        die("invalid packet")

    if cmd != None and data[0] != ord(cmd):
        die("expected %c, got %c" % (cmd, data[0]))

    return data

#------------------------------------------------------------------------------
#   transmit a packet to the target

def tx_packet(port, data):
    size = len(data) + 1
    dbg("tx_packet size %d" % size)
    port.write( [size & 0xFF] )
    port.write( data )

#------------------------------------------------------------------------------
#   upload a file to the target

def upload(port, addr, data):
    log("uploading %d bytes to address %04X" % (len(data), addr))
    while len(data) > 0:
        chunk = data[:252]
        data = data[252:]
        dbg("write %d byte chunk to address %04X" % (len(chunk), addr))
        tx_packet( port, bytes([ ord('W'), addr & 0xFF, addr >> 8 ]) + chunk )
        addr += len(chunk)
        rx_packet(port, 1, '.')
    log("upload complete")

#------------------------------------------------------------------------------
#   download a file to the target

def download(port, addr, size):
    log("downloading %d bytes from address %04X" % (size, addr))
    data = bytearray()
    while size > 0:
        chunk_size = min(size, 255)
        dbg("read %d byte chunk from address %04X" % (chunk_size, addr))
        tx_packet( port, bytes([ ord('R'), addr & 0xFF, addr >> 8, chunk_size ]) )
        data += rx_packet(port, chunk_size)
        addr += chunk_size
        size -= chunk_size
    log("download complete")
    return data

#------------------------------------------------------------------------------
#   trigger reset

def trigger_reset(port):

    #   pulse dtr to reset the board
    port.setDTR(True)
    port.setDTR(False)

    #   read initial 'R' packet
    cmd, version_lo, version_hi =  rx_packet(port, 3, 'R')
    print("tinyload65 %02d.%02d" % (version_hi, version_lo))


#------------------------------------------------------------------------------
#   trigger nmi

def trigger_nmi(port):

    #   pulse rts to trigger nmi
    port.setRTS(False)
    port.setRTS(True)

    #   read 'N' packet
    cmd, reg_lo, reg_hi =  rx_packet(port, 3, 'N')
    addr = reg_lo + (reg_hi << 8)
    dbg("nmi reg %04X" % addr)
    data = download(port, addr, 7)
    reg_sp, reg_y, reg_x, reg_a, reg_st, reg_pcl, reg_pch = data
    print("NMI: SP=%02X Y=%02X X=%02X A=%02X ST=%02X PC=%04X" % (reg_sp, reg_y, reg_x, reg_a, reg_st, reg_pcl + (reg_pch << 8)))

#------------------------------------------------------------------------------
#   process command line arguments

def main(): 
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    def addr_type(x):
        return int(x, 0)

    parser.add_argument(
        "-v", "--verbose", help="verbose level", action='count', default=0
    )
    parser.add_argument(
        "-p", "--port", help="serial port", required=True
    )
    parser.add_argument(
        "-b", "--baud", type=int, help="baud rate", default=57600
    )
    parser.add_argument(
        "-a", "--addr", type=addr_type, help="start address", default=0x1000
    )
    parser.add_argument(
        "-s", "--size", type=int, help="no. of bytes to read", default=256
    )
    parser.add_argument(
        "-u", "--upload", metavar='FILE', help="upload file"
    )
    parser.add_argument(
        "-d", "--download", metavar='FILE', help="download file"
    )
    parser.add_argument(
        "-n", "--nmi", help="trigger nmi", action='store_true'
    )
    
    args = parser.parse_args()
    
    if args.addr < 0x0200 or args.addr > 0xFFFF:
        die("invalid start address %04X" % args.start)
    global verbose 
    verbose = args.verbose

    #   open serial port, avoid initial dtr/rts signals
    port = serial.Serial()
    port.port = args.port
    port.baudrate = args.baud
    port.timeout = 1
    port.dtr = False
    port.rts = False
    port.open()

    if not port.isOpen():
        die("could not open port %s" % args.port)

    if args.nmi:
        trigger_nmi(port)
    else:
        trigger_reset(port)        

    #   upload file
    if ( args.upload ):
        fh = open(args.upload, "rb")
        upload(port, args.addr, fh.read())

    #   download file
    if ( args.download ):
        if args.download == '-':
            fh = os.fdopen(sys.stdout.fileno(), 'wb')
        else:
            fh = open(args.download, 'wb')
        fh.write( download(port, args.addr, args.size) )

#------------------------------------------------------------------------------
#   invoke main() if run as a script

if __name__ == "__main__":
    main()

