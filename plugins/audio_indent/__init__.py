# Audio_indent plugin for 6pad++
import sixpad as sp
from sixpad import msg, window as win
from os import path

pluginpath = sp.appdir + '\\plugins\\audio_indent\\'

def pageKeyUp (page, keycode):
	global pluginpath
	curLine=page.curLine
	if curLine==page.lastLine and keycode!=9: return True
	curIndent=page.lineIndentLevel(curLine)
	if curIndent>page.lastIndent: win.playSound(pluginpath + 'indent.wav')
	elif curIndent<page.lastIndent: win.playSound(pluginpath + 'deindent.wav')
	page.lastLine=curLine
	page.lastIndent=curIndent

def pageOpened (page):
	page.lastLine=-1
	page.lastIndent=0
	page.addEvent('keyUp', pageKeyUp)

win.addEvent('pageOpened', pageOpened)
for page in win.pages: pageOpened(page)

