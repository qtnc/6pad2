# Minide for 6pad++
import re, sixpad as sp
from importlib import import_module
from os import path
from glob import glob
from sixpad import msg, window as win
from .FileType import FileType
from .Project import Project


pluginpath = path.dirname(__file__)

for lang in (sp.locale, 'english'):
	langfile = pluginpath + lang + '.lng'
	if path.isfile(langfile) and sp.loadTranslation(langfile): break

def quickJump (s):
	m = re.match(r'^(.*?)(::|[/:@# ])(.*)$', s)
	if m:
		file, cmd, arg = m.group(1, 2, 3)
		if cmd in quickJumpCommands: return quickJumpCommands[cmd](arg)
	m = re.match('^([-+!])(.*)$', s)
	if m:
		cmd, arg = m.group(1,2)
		if cmd in quickJumpCommands: return quickJumpCommands[cmd](arg)
	win.warning('Unknown command: '+s)

def qjGoToLineColumn (arg):
	p = win.curPage
	m = re.match(r'^(\d+)(?::(\d+))?$', arg)
	if not m: win.warning('Syntax error')
	line, column = (int(x)-1 for x in m.groups(0))
	if column<0: p.position = p.lineSafeStartOffset(line)
	else: p.position = p.lineStartOffset(line) + column

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
	global items
	p = win.curPage
	for item in items:
		item.enabled = hasattr(p, 'project') and hasattr(p.project, item.name) and callable(getattr(p.project, item.name))

def pageOpened (page):
	page.addEvent('activated', pageActivated)
	page.addEvent('beforeSave', pageBeforeSave)
	pageDetectType(page)

menuFormat = win.menus.format
menuProject = win.menus.add(label=msg('&Project'), submenu=True, index=-2)
items = []
for item in (
	{ 'name': 'buildDebug', 'label': 'Build debug', 'accelerator': 'F9' },
	{ 'name': 'buildRelease', 'label': 'Build release', 'accelerator': 'Shift+F9' },
	{ 'name': 'buildTests', 'label': 'Build tests' },
	{ 'name': 'buildDoc', 'label': 'Build documentation' },
	{ 'name': 'clean', 'label': 'Clean' },
	{ 'name': 'update', 'label': 'Update' },
	{ 'name': 'runDebug', 'label': 'Run debug', 'accelerator': 'Ctrl+F9' },
	{ 'name': 'runRelease', 'label': 'Run release', 'accelerator': 'Ctrl+Shift+F9' },
	{ 'name': 'runTests', 'label': 'Run tests' },
	{ 'name': 'deploy', 'label': 'Deploy' },
):
	def f():
		getattr(win.curPage.project, item['name'])()
	items.append( menuProject.add(action=f, **item) )

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


