# -*- coding : utf-8 -*-
import os;
import win32file;
import win32con;
import _thread;
import signal;


def handler(signum, frame):
	if signum == signal.SIGINT:
		print("KeyBoard Interrupt : Ctrl + C --> STOP.");
		os._exit(1);

ACTIONS = { 
1 : "Created", 
2 : "Deleted", 
3 : "Updated", 
4 : "Renamed from something", 
5 : "Renamed to something" 
} 

# FILE_LIST_DIRECTORY = win32con.GENERIC_READ | win32con.GENERIC_WRITE 
# path_to_watch = "." 
# hDir = win32file.CreateFile ( 
# path_to_watch, 
# FILE_LIST_DIRECTORY, 
# win32con.FILE_SHARE_READ | win32con.FILE_SHARE_WRITE, 
# None, 
# win32con.OPEN_EXISTING, 
# win32con.FILE_FLAG_BACKUP_SEMANTICS, 
# None 
# )

def restartzeus(args = None):
	print("try to kill zeus.exe...");
	if os.system("tasklist | find \"zeus.exe\"") :
		os.system("taskkill /f /im zeus.exe");
		os.system("taskkill /f /im cmd.exe");
	print("restarting zeus.exe......",end="");
	if not os.system("start /min \"startzeus\" left_blue.bat") : # /min 最小化执行
		print("Done!");
	else:
		print("Failed!");

def monitor(path_to_watch,function,args=None,mask=None):

	FILE_LIST_DIRECTORY = win32con.GENERIC_READ | win32con.GENERIC_WRITE

	mask =  (mask == None) and win32con.FILE_NOTIFY_CHANGE_FILE_NAME | win32con.FILE_NOTIFY_CHANGE_DIR_NAME | win32con.FILE_NOTIFY_CHANGE_ATTRIBUTES | win32con.FILE_NOTIFY_CHANGE_SIZE | win32con.FILE_NOTIFY_CHANGE_LAST_WRITE | win32con.FILE_NOTIFY_CHANGE_SECURITY or mask;

	hDir = win32file.CreateFile ( path_to_watch, FILE_LIST_DIRECTORY, win32con.FILE_SHARE_READ | win32con.FILE_SHARE_WRITE, None, win32con.OPEN_EXISTING, win32con.FILE_FLAG_BACKUP_SEMANTICS, None );
	while 1:
		results = win32file.ReadDirectoryChangesW(hDir, 1024, True, mask, None, None);
			# hDir,  #handle: Handle to the directory to be monitored. This directory must be opened with the FILE_LIST_DIRECTORY access right. 
			# 1024,  #size: Size of the buffer to allocate for the results. 
			# True,  #bWatchSubtree: Specifies whether the ReadDirectoryChangesW function will monitor the directory or the directory tree.  
		for action, file in results: 
			full_filename = os.path.join (path_to_watch, file) 
			print (full_filename, ACTIONS.get (action, "Unknown"));
			function(args);

def main():
	restartzeus();
	signal.signal(signal.SIGINT, handler);
	filename = "monitor.conf";
	try:
		cfg = open("./" + filename,encoding="utf-8",errors='ignore');
	except FileNotFoundError as err:
		print("can\'t open monitor.conf, please check.");
		os._exit(1);
	pass;

	try:
		for name in cfg:
			if '\n'==name:
				continue;
			if name[-1]=='\n':
				name=name[:-1];
			print("monitor :",name);
			_thread.start_new_thread(monitor,(name,restartzeus));
	except:
		print("Error : unable to start thread.");
	finally:
		cfg.close();
	while 1:
		pass;
pass;#end of main()

if __name__ == '__main__':
	main();
pass;

# mask = ('mask' in dir()) and mask or 1