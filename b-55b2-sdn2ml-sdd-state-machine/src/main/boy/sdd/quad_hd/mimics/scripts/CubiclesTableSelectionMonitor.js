importClass(Packages.org.csstudio.opibuilder.scriptUtil.DataUtil);
importClass(Packages.org.csstudio.opibuilder.scriptUtil.PVUtil);
importClass(Packages.org.csstudio.opibuilder.scriptUtil.ScriptUtil);
importClass(Packages.org.csstudio.opibuilder.scriptUtil.ConsoleUtil);

var table = widget.getTable();
var fct_name=widget.getPropertyValue("name");
var selectionChanged  = new Packages.org.csstudio.swt.widgets.natives.SpreadSheetTable.ITableSelectionChangedListener() {
    selectionChanged: function(selection) {
 		
    	var selectedrow=  table.getSelection();
    	var cuIndex=selectedrow[0][0];
		var phyName=selectedrow[0][1];
		var loc=selectedrow[0][5];
		var pppp=selectedrow[0][6];
		var pp=selectedrow[0][7];
		var nnnn=selectedrow[0][8];

		var macroInput = DataUtil.createMacrosInput(true)
		macroInput.put("CUB", cuIndex)
		macroInput.put("PHY_NAME", phyName)
		macroInput.put("FCT_NAME", fct_name)
		macroInput.put("PPPP", pppp)
		macroInput.put("PP", pp)
		macroInput.put("NNNN", nnnn)
		macroInput.put("CUB_LOC", "Location: "+loc)
        		
		ScriptUtil.openOPI(widget, fct_name+"-CubicleDetails.opi", 1, macroInput);
	}
};
table.addSelectionChangedListener(selectionChanged);
