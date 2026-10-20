importClass(Packages.org.csstudio.opibuilder.scriptUtil.DataUtil);
importClass(Packages.org.csstudio.opibuilder.scriptUtil.PVUtil);
importClass(Packages.org.csstudio.opibuilder.scriptUtil.ScriptUtil);
importClass(Packages.org.csstudio.opibuilder.scriptUtil.ConsoleUtil);

var table = widget.getTable();
var fct_name=display.getPropertyValue("name");
var selectionChanged  = new Packages.org.csstudio.swt.widgets.natives.SpreadSheetTable.ITableSelectionChangedListener() {
    selectionChanged: function(selection) {
		var macroInput = DataUtil.createMacrosInput(true)
		ScriptUtil.openOPI(display.getWidget("GeneralTable"), ${CODAC_ROOT}+"/opi/epics-sysmon/boy/ITER-SYSM-PLC_Mimic.opi", 1, macroInput);
	}
};
table.addSelectionChangedListener(selectionChanged);
