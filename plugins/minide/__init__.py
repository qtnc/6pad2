# Minide for 6pad++
import re, os, sixpad as sp, qc6paddlgs as dialogs
from importlib import import_module
from os import path
from glob import glob
from fnmatch import fnmatch
from sixpad import msg, window as win
from .FileType import FileType
from .Project import Project


pluginpath = path.dirname(__file__)

excludeFiles = (
	'*.exe', '*.dll', '*.pyd', '*.pyc', '*.pyo', '*.lc', '*.bin', '*.o', 
	'*.class', '*.jar', '*.war', '*.ear', '*.zip', '*.7z',
	'*.wav', '*.mp3', '*.ogg', '*.png', '*.gif', '*.jpg',
)

for lang in (sp.locale, 'english'):
	langfile = pluginpath + lang + '.lng'
	if path.isfile(langfile) and sp.loadTranslation(langfile): break

def quickJump (s):
	m = re.match(r'^(.*?)(::|[/:@# ])(.*)$', s)
	if m:
		file, cmd, arg = m.group(1, 2, 3)
		if cmd in quickJumpCommands: return qjGoToFile(file) and quickJumpCommands[cmd](arg)
	m = re.match('^([-+!])(.*)$', s)
	if m:
		cmd, arg = m.group(1,2)
		if cmd in quickJumpCommands: return quickJumpCommands[cmd](arg)
	win.warning('Unknown command: '+s)

def qjGoToFile (file):
	if not file: return True
	for page in win.pages:
		if page.file and qjFileMatches(page.file, file):
			page.focus()
			return True
	if win.curPage.project and win.curPage.project.dir: return qjGoToFileRec(win.curPage.project.dir, file)
	return False

def qjGoToFileRec (dir, pattern):
	dirs=[]
	for file in os.listdir(dir):
		file = path.join(dir,file)
		if path.isdir(file):
			dirs.append(file)
		elif qjFileMatches(file, pattern) and not any(fnmatch(path.basename(file), x) for x in excludeFiles):
			page = win.open(file)
			if page:
				page.focus()
				return True
	for dir in dirs:
		if qjGoToFileRec(dir, pattern): return True
	return False

def qjFileMatches (file, pattern):
	return fnmatch(path.basename(file), pattern+'*') or fnmatch(file, pattern+'*')

def qjGoToLineColumn (arg):
	p = win.curPage
	m = re.match(r'^(\d+)(?::(\d+))?$', arg)
	if not m: win.warning('Syntax error')
	line, column = (int(x)-1 for x in m.groups(0))
	if column<0: p.position = p.lineSafeStartOffset(line)
	else: p.position = p.licol(line, column)

def qjIncLineNum (arg):
	win.curPage.curLine += int(arg)

def qjDecLineNum (arg):
	win.curPage.curLine -= int(arg)

def qjFindLit (arg):
	win.curPage.find(arg, stealthty=True)

def qjFindReg (arg):
	win.curPage.find(arg, regex=True, stealthty=True)
	
def detectProjectType (dir):
	for pd in Project.detectors:
		project = pd(dir)
		if project is None: continue
		if project.file in Project.projects: return Project.projects[project.file]
		Project.projects[project.file] = project
		return project

def pageDetectFileType (file):
	for ftd in FileType.detectors:
		ft = ftd(file)
		if ft is not None: return ft

def pageDetectType (page):
	global typeAliases, pluginpath
	page.lastFile = page.file
	page.fileType = pageDetectFileType(page.file)
	dir = path.dirname(path.realpath(page.file))
	while (not hasattr(page, 'project') or page.project is None) and len(dir)>3:
		page.project = detectProjectType(dir)
		if not page.project: dir = path.dirname(dir[:-1])
	print('FileType = ', page.fileType)
	print('Project = ', page.project)

def pageBeforeSave (page, file):
	if file!=page.lastFile: pageDetectType(page)

def pageActivated (page):
	pass

def pageOpened (page):
	page.addEvent('activated', pageActivated)
	page.addEvent('beforeSave', pageBeforeSave)
	pageDetectType(page)

menuFormat = win.menus.format
menuProject = win.menus.add(label=msg('&Project'), submenu=True, index=-2, name='project')

quickJumpCommands = {
	':': qjGoToLineColumn,
	'+': qjIncLineNum,
	'-': qjDecLineNum,
	'/': qjFindReg,
	' ': qjFindLit,
}

for file in glob(pluginpath + '/ProjectTypes/*.py') + glob(pluginpath + '/FileTypes/*.py'):
	if file.endswith('__init__.py'): continue
	import_module('.' + path.relpath(file, pluginpath) .replace('\\', '.') [:-3], __name__)

win.addEvent('quickJump', quickJump)
win.addEvent('pageOpened', pageOpened)
for page in win.pages: pageOpened(page)
pageActivated(win.curPage)


