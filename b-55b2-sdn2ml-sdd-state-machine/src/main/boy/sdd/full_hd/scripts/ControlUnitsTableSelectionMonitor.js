
importClass(Packages.org.csstudio.swt.widgets.natives.SpreadSheetTable.ITableSelectionChangedListener);
importPackage(Packages.org.csstudio.opibuilder.scriptUtil);

var table = widget.getTable();
var fct_name=widget.getPropertyValue("name");

var selectionChanged  = new Packages.org.csstudio.swt.widgets.natives.SpreadSheetTable.ITableSelectionChangedListener() {
    selectionChanged: function(selection) {
    	var selectedrow=  table.getSelection();
    	var cuName=selectedrow[0][0];
		var phyName=selectedrow[0][1];
    	var plcIocHlts =selectedrow[0][6];
        var cuType=selectedrow[0][7];
// change $(CU) substitution
        macroInput = DataUtil.createMacrosInput(true)
        macroInput.put("CU", cuName)
        macroInput.put("PHY_NAME", phyName)
        macroInput.put("FCT_NAME", fct_name)
        macroInput.put("CU_TYPE", cuType)
        if (plcIocHlts == "") {
        	macroInput.put("SHOW_PLC_IOC", "false")
        }
        else {
        	macroInput.put("SHOW_PLC_IOC", "true")
        }
        // open OPI
        // see https://svnpub.iter.org/codac/iter/codac/dev/units/m-css-boy/trunk/org.csstudio.opibuilder/src/org/csstudio/opibuilder/scriptUtil/ScriptUtil.java
        if (cuType == "POC with CA") {
            ScriptUtil.openOPI(widget, fct_name + "-" + cuName + "-POCWithCADetails.opi", 1, macroInput)
        }
        else if (cuType == "POC without CA") {
            ScriptUtil.openOPI(widget, fct_name + "-" + cuName + "-POCWithoutCADetails.opi", 1, macroInput)
        }
        else if (cuType == "Plant System Host") {
            ScriptUtil.openOPI(widget, fct_name + "-" + cuName + "-PSHDetails.opi", 1, macroInput)
        }
        else if (cuType == "Fast Controller") {
            ScriptUtil.openOPI(widget, fct_name + "-" + cuName + "-PCFDetails.opi", 1, macroInput)
        }
        else if (cuType == "Server") {
            ScriptUtil.openOPI(widget, fct_name + "-" + cuName + "-SRVDetails.opi", 1, macroInput)
        }
        else {
            ScriptUtil.openOPI(widget, fct_name+"-CtrlUnitDetails.opi", 1, macroInput)
        }
	
    }
};

table.addSelectionChangedListener(selectionChanged);
