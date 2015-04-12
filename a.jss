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
EndScript

Script AltRightArrow ()
TypeCurrentScriptKey();
EndScript

Script OpenListBox ()
if GetObjectTypeCode() == WT_EDIT then
TypeCurrentScriptKey()
Else
PerformScript OpenListBox();
EndIf
EndScript

Script CloseListBox ()
if GetObjectTypeCode() == WT_EDIT then
TypeCurrentScriptKey()
Else
PerformScript CloseListBox();
EndIf
EndScript
