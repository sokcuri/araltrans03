
String.prototype.trim = function(){	return this.replace(/(^\s*)|(\s*$)/gi,"");}

// Get HTML Object
function getObject(objectId) 
{ 
	// checkW3C DOM, then MSIE 4, then NN 4. 
	if(document.getElementById && document.getElementById(objectId))
		return document.getElementById(objectId); 

	else if (document.all && document.all(objectId)) 
		return document.all(objectId); 

	else if (document.layers && document.layers[objectId]) 
		return document.layers[objectId];

	else
		return false; 
} 

function getAT3Dir()
{
	var strAT3Dir = document.location.pathname;
	strAT3Dir = strAT3Dir.substr(0, strAT3Dir.lastIndexOf("\\"));
	strAT3Dir = strAT3Dir.substr(0, strAT3Dir.lastIndexOf("\\"));
	strAT3Dir = strAT3Dir.substr(0, strAT3Dir.lastIndexOf("\\"));
	return strAT3Dir;
}

// encodePath
function encodePath(srcPath, strAppDir)
{
	var strRetVal = srcPath;

	var strAT3Dir = getAT3Dir().toUpperCase();
	if(strRetVal.toUpperCase().indexOf(strAT3Dir) == 0)
	{
		strRetVal = "\%AT3Dir\%" + strRetVal.substr(strAT3Dir.length);
	}

	if(strAppDir && strAppDir.length > 0)
	{
		if(strRetVal.toUpperCase().indexOf(strAppDir.toUpperCase()) == 0)
		{
			strRetVal = "\%AppDir\%" + strRetVal.substr(strAppDir.length);
		}
	}

	return strRetVal;
}

// decodePath
function decodePath(srcPath, strAppDir)
{
	var strRetVal = srcPath;

	var strAT3Dir = getAT3Dir();
	strRetVal = strRetVal.replace("\%AT3Dir\%", strAT3Dir);

	if(strAppDir && strAppDir.length > 0)
	{
		strRetVal = strRetVal.replace("\%AppDir\%", strAppDir);
	}

	return strRetVal;
}

// isNumeric
function isNumeric(sText)
{
   if(sText == null || sText.length == 0) return false;
   
	var ValidChars = "0123456789.";
	var Char;

	for (i = 0; i < sText.length; i++) 
	{ 
		Char = sText.charAt(i); 
		if (ValidChars.indexOf(Char) == -1) 
		{
			return false;
		}
	}

	return true;
}



// Browse Folder
function browseFolder(description)
{
	// Accessing the Shell:
	var objShell = new ActiveXObject("Shell.Application");

	// Calling the browser dialog:
	if(!description) description = "Select a folder";
	var objFolder = objShell.BrowseForFolder(0, description, 0);

	if (objFolder == null) return "";

	// Accessing the folder through FolderItem object:
	var objFolderItem = objFolder.Items().Item();
	var objPath = objFolderItem.Path;

	var foldername = objPath;
	if (foldername.substr(foldername.length-1, 1) == "\\")
		foldername = foldername.substr(0, foldername.length-1);

	return foldername;
}

// Get Text of Child Element
function getChildElemText(elem, childName)
{ 
	try
	{
		return elem.getElementsByTagName(childName)[0].text;
	}
	catch(ex)
	{
		return null;
	}
} 

// applySetting
function applySetting(objATContainer, elemPluginList, elemContextList)
{
	var bRetVal = true;

	// Read 'Plugin' elements from 'PluginList'
	try
	{
		var elemcolPlugin = elemPluginList.getElementsByTagName("Plugin");
		var pluginCnt = elemcolPlugin.length;
		for(var i = 0; i < pluginCnt; i++)
		{
			var elemPlugin = elemcolPlugin[i];
			var strPluginFileName = getChildElemText(elemPlugin, "PluginFileName");
			var strPluginFileFolder = decodePath(getChildElemText(elemPlugin, "PluginFileFolder"));
			//var strPluginFileHash = getChildElemText(elemPlugin, "PluginFileHash");
			//var strPluginFileVersion = getChildElemText(elemPlugin, "PluginFileVersion");
			//var strDownloadUrl = getChildElemText(elemPlugin, "DownloadUrl");
			//var strPluginName = getChildElemText(elemPlugin, "PluginName");
			//var strPluginType = getChildElemText(elemPlugin, "PluginType");
			var strPluginOption = getChildElemText(elemPlugin, "PluginOption");

			//alert(objATContainer)
			//alert(strPluginFileFolder + "\\" + strPluginFileName + ", '" + strPluginOption + "'");
			objATContainer.LoadPlugin(strPluginFileFolder + "\\" + strPluginFileName, strPluginOption);
		}
	}
	catch(ex)
	{
		bRetVal = false;
	}
		
	
	// Read 'Context' elements from 'ContextList'
	try
	{
		var elemcolContext = elemContextList.getElementsByTagName("Context");
		var contextCnt = elemcolContext.length;
		for(var i = 0; i < contextCnt; i++)
		{
			var elemContext = elemcolContext[i];
			var strContextName = getChildElemText(elemContext, "ContextName");
			
			// Read 'Object' elements from 'ObjectList'
			var nContextIdx = objATContainer.FindContextIndexByName(strContextName);
			var objContext = objATContainer.GetContext(nContextIdx);
			var elemcolObject = elemContext.getElementsByTagName("ObjectList")[0].getElementsByTagName("Object");
			var objectCnt = elemcolObject.length;
			for(var j = 0; j < objectCnt; j++)
			{
				var elemObject = elemcolObject[j];
				var strObjectName = getChildElemText(elemObject, "ObjectName");
				var strObjectOption = getChildElemText(elemObject, "ObjectOption");
				//alert(strObjectName + ", " + j + ", " + strObjectOption);
				objContext.AddTransObject(strObjectName, j, strObjectOption);
			}

		}
	}
	catch(ex)
	{
		bRetVal = false;
	}

	return bRetVal;
}

// Check AralTrans Update
function checkAralTransNewVersion()
{
	var bRetVal = false;
	//var objAralTransApp = new ActiveXObject("AralTrans.Application");

	var objShell = new ActiveXObject("WScript.Shell");
	var nRes = objShell.run("../../AralUpdater.exe check http://www.aralgood.com/update_files_AT3/AralTrans3Update.ini", 1, true);
	if(nRes == -1) bRetVal = true;

	return bRetVal;
}

// Perform AralTrans Update
function updateAralTrans()
{
	var bRetVal = false;
	var objAralTransApp = new ActiveXObject("AralTrans.Application");
	if(objAralTransApp)
	{
		objAralTransApp.Exit();
	}

	var objShell = new ActiveXObject("WScript.Shell");
	var nRes = objShell.run("../../AralUpdater.exe execute http://www.aralgood.com/update_files_AT3/AralTrans3Update.ini", 1, true);
	if(nRes != -1) bRetVal = true;

	return bRetVal;
}

/*
function showOpen()
{
	var strResult = null;

	var objDialog = new ActiveXObject("MSComDlg.CommonDialog");
	objDialog.DialogTitle = "SaveAs";
	objDialog.Filter = "Scripts|*.vbs;*.hta;*.wsf;*.js|Text Files|*.txt|All files|*.*";
	objDialog.MaxFileSize = 255;
	var intResult = objDialog.ShowSave();
	if(intResult != 0)
	{
		strResult = objDialog.FileName;
	}

	return strResult;
}
*/

function showOpen(filter, defaultpath)
{
	var strResult = null;

	try
	{
		var cdlCancel = 32755;
		var cdlOFNOverwritePrompt = 2;
		var cdlOFNHideReadOnly = 4;
		var cdlOFNNoChangeDir = 8;
		var cdlOFNAllowMultiselect = 512;
		var cdlOFNFileMustExist = 4096;
		var cdlOFNExplorer = 524288;

		var objDialog = new ActiveXObject("MSComDlg.CommonDialog.1");
		objDialog.Flags = cdlOFNHideReadOnly + cdlOFNExplorer + cdlOFNFileMustExist + cdlOFNNoChangeDir;  
		objDialog.DialogTitle = "Open";
		if(filter && filter != "")
		{
			objDialog.Filter = filter;
		}
		else
		{
			objDialog.Filter = "All files (*.*)|*.*||";
		}

		objDialog.InitDir = "";
		objDialog.FileName = "";
		objDialog.FilterIndex = 0;
		if(defaultpath && defaultpath != "")
		{
			fso = new ActiveXObject("Scripting.FileSystemObject");
			if(fso.FolderExists(defaultpath))
			{
				objDialog.InitDir = defaultpath;
			}
			else
			{
				var nStrIdx = defaultpath.lastIndexOf('\\');
				if(nStrIdx != -1)
				{
					objDialog.InitDir = defaultpath.substr(0, nStrIdx);
					if(nStrIdx+1 < defaultpath.length) objDialog.FileName = defaultpath.substr(nStrIdx+1);
				}
			}
		}

		objDialog.MaxFileSize = 8000;
		objDialog.CancelError = true;
		try
		{
			objDialog.ShowOpen();
			strResult = objDialog.FileName;
		}
		catch(ex){}
	}
	catch(NotSupportComdlg32)
	{
		if(!filter || filter == "")
		{
			filter = "All files (*.*)|*.*||";
		}
		var objAralTransApp = new ActiveXObject("AralTrans.Application");
		strResult = objAralTransApp.ShowFileDialog(true, filter, defaultpath);
	}

	return strResult;
}

function showSave(filter, defaultpath)
{
	var strResult = null;

	try
	{
		var cdlCancel = 32755;
		var cdlOFNOverwritePrompt = 2;
		var cdlOFNHideReadOnly = 4;
		var cdlOFNNoChangeDir = 8;
		var cdlOFNAllowMultiselect = 512;
		var cdlOFNFileMustExist = 4096;
		var cdlOFNExplorer = 524288;

		var objDialog = new ActiveXObject("MSComDlg.CommonDialog.1");
		objDialog.Flags = cdlOFNHideReadOnly + cdlOFNExplorer + cdlOFNOverwritePrompt + cdlOFNNoChangeDir;
		objDialog.DialogTitle = "Save";
		if(filter && filter != "")
		{
			objDialog.Filter = filter;
		}
		else
		{
			objDialog.Filter = "All files (*.*)|*.*||";
		}
		objDialog.FilterIndex = 0;

		objDialog.InitDir = "";
		objDialog.FileName = "";
		if(defaultpath && defaultpath != "")
		{
			fso = new ActiveXObject("Scripting.FileSystemObject");
			if(fso.FolderExists(defaultpath))
			{
				objDialog.InitDir = defaultpath;
			}
			else
			{
				var nStrIdx = defaultpath.lastIndexOf('\\');
				if(nStrIdx != -1)
				{
					objDialog.InitDir = defaultpath.substr(0, nStrIdx);
					if(nStrIdx+1 < defaultpath.length) objDialog.FileName = defaultpath.substr(nStrIdx+1);
				}
			}
		}

		objDialog.MaxFileSize = 8000;
		objDialog.CancelError = true;
		try
		{
			objDialog.ShowSave();
			strResult = objDialog.FileName;
		}
		catch(ex){}
	}
	catch(NotSupportComdlg32)
	{
		if(!filter || filter == "")
		{
			filter = "All files (*.*)|*.*||";
		}
		var objAralTransApp = new ActiveXObject("AralTrans.Application");
		strResult = objAralTransApp.ShowFileDialog(false, filter, defaultpath);
	}


	return strResult;
}