TEMPLATE = subdirs

SUBDIRS += bowlib testGUI

testGUI.depends = bowlib
