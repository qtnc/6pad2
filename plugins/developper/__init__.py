# Developper plugin for 6pad++
import os, importlib, sixpad as sp
from sixpad import msg, window as win
from os import path

pluginpath = sp.appdir + '\\plugins\\developper\\'
typeAliases = {
'config':'ini', 'properties':'ini', 'conf':'ini', 'cfg':'ini', 'cnf':'ini', 'inf':'ini',
'md':'markdown', 'txt':'md', 'mmd':'md', 
'h':'cpp', 'hpp':'cpp', 'hxx':'cpp', 'c':'cpp', 'cc':'cpp', 'cxx':'cpp', 'tcc':'cpp', 'tpp':'cpp',
'res':'winres', 'rc':'winres', 'reg':'winreg',
'py':'python', 'pyw':'py',
'htm':'html',
'opf':'xml', 'xsl':'xml', 'xslt':'xml', 'xsd':'xml', 'rss':'xml', 'rdf':'xml', 'svg':'xml', 'xpf':'xml',
'vbs':'vbscript', 'js':'javascript',
'inc':'php',
'bat':'winbatch',
'sh':'shell',
'pl':'perl',
'bas':'basic',
'pas':'pascal',
}
prjAliases = {
'c.bat': 'qcprj',
'makefile': 'makefile'
}

for lang in (sp.locale, 'english'):
	langfile = pluginpath + lang + '.lng'
	if path.isfile(langfile) and sp.loadTranslation(langfile): break

def detectProjectType (dir):
	global prjAliases
	for file in prjAliases:
		if path.isfile(dir + '\\' + file): return prjAliases[file]

def pageDetectType (page):
	global typeAliases, pluginpath
	page.lastFile = page.file
	ext = path.splitext(page.file)[1][1:].lower()
	while ext in typeAliases: ext = typeAliases[ext]
	if path.isfile(pluginpath + ext + '.py'):
		mod = importlib.import_module('developper.' + ext)
		if hasattr(mod, 'reindent'): page.reindent = mod.reindent
	dir = path.dirname(path.realpath(page.file))
	while not hasattr(page, 'project') and len(dir)>3:
		prj = detectProjectType(dir)
		if prj and path.isfile(pluginpath + prj + '.py'):
			mod = importlib.import_module('developper.'+prj)
			page.project = mod.Project(dir)
		dir = path.dirname(dir)

def pageBeforeSave (page, file):
	if file!=page.lastFile: pageDetectType(page)

def pageActivated (page):
	global itemReindent, itemCompile, itemRun
	p = win.curPage
	itemReindent.enabled = hasattr(p, 'reindent') and callable(p.reindent)
	itemCompile.enabled = hasattr(p, 'project') and hasattr(p.project, 'compile') and callable(p.project.compile)
	itemRun.enabled = hasattr(p, 'project') and hasattr(p.project, 'run') and callable(p.project.run)

def pageOpened (page):
	page.addEvent('activated', pageActivated)
	page.addEvent('beforeSave', pageBeforeSave)
	pageDetectType(page)

def reindent ():
	win.curPage.reindent(win.curPage)

def compile ():
	win.curPage.project.compile(win.curPage)

def execute ():
	win.curPage.project.run(win.curPage)

menuFormat = win.menus.format
menuProject = win.menus.add(label=msg('&Project'), submenu=1, index=-2)
itemReindent = menuFormat.add(action=reindent, label=msg('Re&indent'), name='reindent', index=-2, accelerator='Ctrl+I')
itemCompile = menuProject.add(action=compile, label=msg('&Compile'), name='compile', accelerator='F9')
itemRun = menuProject.add(action=execute, label=msg('&Run'), name='run', accelerator='Ctrl+F9')

def f(self): print(self)
win.curPage.reindent = f

win.addEvent('pageOpened', pageOpened)
for page in win.pages: pageOpened(page)
pageActivated(win.curPage)
