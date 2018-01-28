#! /usr/bin/python

#################################################################################
#                                                                               #
#	Serial interface with Arduino Python script.                                #
#	Created by Manuel Montenegro, January 26, 2017.                             #
#	Developed for Manuel Montenegro Bachelor Thesis.                            #
#                                                                               #
#	This is an early version of user interface for administrators that want     #
#	to manage a sport event.                                                    #
#                                                                               #
#################################################################################

import serial
import time
import sys



# Main function. This will execute at the beginning of the process
def main():

	# Serial connection with Arduino parameters
	port = '/dev/cu.usbmodem1431'
	baudrate = 115200

	# Starts the serial connection
	arduino = serial.Serial(port, baudrate)
	# Waits for stablish connection
	time.sleep(2)

	introMenu(arduino);

	arduino.close();



# Introduction menu for Master device
def introMenu (arduino):

	choice = None

	while (choice != '0'):
		# Prints the options user can choose
		print "\n\n\n --------------------------------------------"
		print " | MASTER device for setting up sport event |"
		print " --------------------------------------------"
		print " Please, send the option number of your choice"
		print "   1. Set up and start a new event"
		print "   2. Continue setting up an event"
		print "   3. Clean card"
		print "   4. Read card"
		print "   0. Close\n"

		choice = raw_input("Introduce your choice: ")
		arduino.write(bytes(choice))

		ack = arduino.readline().rstrip()

		if ack == '1':

			if choice == '1':

				# Send the time and date
				arduino.write(bytes(time.strftime("%b %d %Y")))
				arduino.write(bytes(time.strftime("%H:%M:%S")))

				setupMenu(arduino)

			elif choice == '2':
				setupMenu(arduino)

			elif choice == '3':
				formatMenu(arduino)



# Menu for set up a station
def setupMenu (arduino):

	choice = '1';

	while choice == '1':

		# Read the station ID
		station = arduino.readline().rstrip()

		print "\n Put station #%s on card reader" % station
		print " After that, send the option number of your choice"
		print "   1. Set up this station"
		print "   2. Finish the setup process\n"

		choice = raw_input("Introduce your choice: ")
		arduino.write(bytes(choice))

		ack = arduino.readline().rstrip()

		if ack == '0':
			print "Error with Station. Reset it and try again"




# Menu for formatting a user card
def formatMenu(arduino):

	print "\nPlease, put card on reader\n"

	# Prints data from card
	uid = arduino.readline().rstrip()
	userName = arduino.readline().rstrip()
	category = arduino.readline().rstrip()

	print " --------------------------------"
	print " User ID: %s" % uid
	print " Name: %s" % userName
	print " Category: %s" % category
	print " --------------------------------"

	print " What do you want to do?"
	print "   1. Change name and category & format card"
	print "   2. Format card with same name and category"
	print "   3. Don't do anything"

	# Ask user for the choice
	choice = raw_input("Introduce your choice: ")
	arduino.write(bytes(choice))

	# If user wants change its name and category
	if choice == '1':
		userName = raw_input("Introduce user name: ")
		arduino.write(bytes(userName))
		category = raw_input("Introduce category: ")
		arduino.write(bytes(category))



# Start process 
if __name__ == '__main__':
    main()