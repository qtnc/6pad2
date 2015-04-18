include  "hjconst.jsh";

Script ControlDownArrow ()
TypeCurrentScriptKey();
SayLine();
EndScript

Script ControlUpArrow ()
TypeCurrentScriptKey();
SayLine();
EndScript

Script AltLeftArrow ()
TypeCurrentScriptKey();
SayLine();
EndScript

Script AltRightArrow ()
TypeCurrentScriptKey();
SayLine();
EndScript

Script MouseRight ()
TypeCurrentScriptKey();
performscript SaySelectedText();
EndScript

Script MouseLeft ()
TypeCurrentScriptKey();
performscript SaySelectedText();
EndScript

Script MouseUp ()
TypeCurrentScriptKey();
performscript SaySelectedText();
EndScript

Script MouseDown ()
TypeCurrentScriptKey();
performscript SaySelectedText();
EndScript

Script OpenListBox ()
if GetObjectTypeCode() == WT_EDIT then
TypeCurrentScriptKey()
SayLine();
Else
PerformScript OpenListBox();
EndIf
EndScript

Script CloseListBox ()
if GetObjectTypeCode() == WT_EDIT then
TypeCurrentScriptKey()
SayLine();
Else
PerformScript CloseListBox();
EndIf
EndScript
