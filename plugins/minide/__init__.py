# Minide for 6pad++
import os, importlib, sixpad as sp
from sixpad import msg, window as win
from os import path

pluginpath = sp.appdir + '\\plugins\\minide\\'


for lang in (sp.locale, 'english'):
	langfile = pluginpath + lang + '.lng'
	if path.isfile(langfile) and sp.loadTranslation(langfile): break

class Project:
	detectors = []
	projects = {}
	
	def detector (d):
		Project.detectors.append(d); return d
	
	def __init__ (self, id, dir):
		self.id=id; self.dir=dir

def detectProjectType (dir):
	for pd in Project.detectors:
		project = pd(dir)
		if project is None: continue
		if project.id in Project.projects: return Project.projects[project.id]
		Project.projects[project.id] = project
		return project

def pageDetectType (page):
	global typeAliases, pluginpath
	page.lastFile = page.file
	dir = path.dirname(path.realpath(page.file))
	while (not hasattr(page, 'project') or page.project is None) and len(dir)>3:
		page.project = detectProjectType(dir)
		if not page.project: dir = path.dirname(dir[:-1])

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
menuProject = win.menus.add(label=msg('&Project'), submenu=1, index=-2)
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

__all__ = [x[:-3] for x in os.listdir(pluginpath) if x[-3:]=='.py' and x!='__init__.py']
from . import *

win.addEvent('pageOpened', pageOpened)
for page in win.pages: pageOpened(page)
pageActivated(win.curPage)

