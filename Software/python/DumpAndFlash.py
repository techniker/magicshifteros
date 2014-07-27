import os
from MagicTest import *


if __name__ == '__main__':
	shifterNr = -1
	doDump = True
	doFlash = True	
	doBitmaps = True
	device = ""	

	if (len(sys.argv) >= 2):
		shifterNr = int(sys.argv[1])
	if (len(sys.argv) >= 3):
		device = sys.argv[2]
	if (len(sys.argv) >= 5):
		doDump = int(sys.argv[3])
		doFlash = int(sys.argv[4])
	if (len(sys.argv) < 2):
		print "usage:"
		print "python DumpAndFlash.py nrOfShifter [serialdevice] [doDump (default 1)] [doFlash (default 1)]"
		print "example: DumpAndFlash.py 123 1 1"
		print "the aboce command line would dump and flash the shifters using serial number 123 (increasing it 1 for every shifter"
		exit()

	if (not doDump) and (not doFlash):
		print "don't dump and don't flash...nothing to do" 
		exit()

	while (1):
		pOK("Ready to dump and flash MagicShifter Nr: " + str(shifterNr) + " (ESC to cancel)")
		key = read_single_keypress()
		if (ord(key) == 27):
			exit()

		if (doDump):
			continueDump = 3
		else:
			continueDump = 0
		while (continueDump):
			fi, fo, fe= os.popen3("python MagicTest.py dump " + device)
			stderr = fe.readlines()
			stdout = fo.readlines()
			print stdout[0]
			if (stdout[0].find("NR_COLOR_TESTS") != -1 and stdout[0].find("ACC") != -1 and stdout[0].find("EOD") != -1):
				dumpName = "./dumpof500/shifter_%04d.txt" % (shifterNr)
				pOK("writing dump file: " + dumpName)
				if (os.path.exists(dumpName)):
					pFail("Dump file already exists! " + dumpName)
					exit()
				else:
					file = open(dumpName, 'wt')
					file.write(stdout[0])
					file.close()
					pOK("writing dump complete")
				continueDump = 0
			else:
				pFail("dump not working, press key to continue esc to stop")
				key = read_single_keypress()
				if (ord(key) == 27):
					exit()
				continueDump = continueDump - 1

		dmesgClear()

		if doFlash:
			print "--> Press the RESET button"
			cnt = 0
			while (not dmesgPing()):
					sleep(0.1)
					cnt = cnt + 1
					if (cnt > 1000):
						pFail("Reset Button was not pressed within 100 seconds. Is it broken?")
						exit()
			pOK("Reset Button OK!")
			sleep(1)
			pOK("programming device...")
			if(callProgrammer()):
				pOK("bootloader OK! :)")
			else:
				pFail("USB connection does not work or Bootloader is not responding. :(")
				exit()
			sleep(1)
		
		if (doBitmaps):
			continueBitmaps = 3
			while (continueBitmaps):
				print "Uploading Bitmaps (takes some seconds be patient)"
				fi, fo, fe = os.popen3("./UploadCree.sh " + device)
				#fi, fo, fe = os.popen3("./UploadCree_Makerbot.sh")
				#fi, fo, fe = os.popen3("./UploadCree_Makerbot_small.sh")
				stderr = fe.readlines()
				stdout = fo.readlines()
				print stdout
				countBitmaps = 0
				for line in stdout:
					countBitmaps += line.count("READBACK @sector")
				if (countBitmaps != 18):
					pFail("OH noes uploading bitmaps failed (enter to continue exc to exit) == " + str(countBitmaps))
					key = read_single_keypress()
					if (ord(key) == 27):
						exit()
					continueBitmaps = continueBitmaps - 1;
				else:
					continueBitmaps = 0
			

		shifterNr += 1
