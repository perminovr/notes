#!/usr/bin/python3

import time

# GUI (python3-tk) ************************************************************************************************
def gui_progress():
	import PySimpleGUI as sg

	mylist = [1,2,3,4,5,6,7,8]
	for i, item in enumerate(mylist):
		sg.one_line_progress_meter('This is my progress meter!', i+1, len(mylist), '-key-')
		time.sleep(1)


# tqdm (can be used in shell (cmd | python3 -n tqdm | wc -l)) *****************************************************
def tqdm_progress():
	from tqdm import tqdm

	mylist = [1,2,3,4,5,6,7,8]
	pbar = tqdm(mylist)
	pbar.set_description("Processing %s" % 'a')
	for i in pbar:
		pbar.write("123")
		time.sleep(1)
	pbar.close()


# progress.bar ****************************************************************************************************
def bar_progress():
	#from progress.bar import ChargingBar
	#from progress.bar import FillingSquaresBar
	#from progress.bar import FillingCirclesBar
	#from progress.bar import IncrementalBar
	#from progress.bar import PixelBar
	#from progress.bar import ShadyBar
	from progress.bar import Bar
	from sys import stdout

	mylist = [1,2,3,4,5,6,7,8]
	bar = Bar('Loading', fill='@', suffix='%(percent)d%% - %(elapsed)ds', max = len(mylist), file=stdout)
	for item in mylist:
		bar.writeln("123")
		print("")
		bar.next()
		time.sleep(1)
	bar.finish()



if __name__ == "__main__":
	bar_progress()


