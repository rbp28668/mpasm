String.prototype.right = function(n) {
	if (n <= 0)     // Invalid bound, return blank string
		return "";
	else if (n > this.length)   // Invalid bound, return
		return this;                     // entire string
	else { // Valid bound, return appropriate substring
		var l = this.length;
		return this.substring(l, l - n);
		}
	}


function doStartEnd(anAreaID, startTxt, midTxt, endTxt ) {
	anArea = (typeof(anAreaID) == "string" ? document.getElementById(anAreaID) : anAreaID);
	window.status += startTxt;
	if (document.selection) {
		anArea.focus();
		var aSel = ( anArea.caretPos ? anArea.caretPos : document.selection.createRange() );
		if (aSel.text.charAt(aSel.text.length-1)==' ') aSel.moveEnd('character',-1);
		if (aSel.text=='') { 
			aSel.moveStart('character',-100000);
			if (aSel.text.lastIndexOf(startTxt) > aSel.text.lastIndexOf(endTxt)) startTxt=midTxt=''; else endTxt='';
			aSel.collapse(false);
			}
		aSel.text = startTxt + midTxt + aSel.text + endTxt;
		if (midTxt.length>2&&endTxt.length==0) {aSel.move("character",-2); aSel.select();}
		aSel.collapse(false);aSel.select();
		}
	else if (anArea.selectionStart && anArea.selectionStart != "undefined") {
		// Mozilla text range replace.
		var text = new Array();
		text[0] = anArea.value.substr(0, anArea.selectionStart);
		text[1] = anArea.value.substr(anArea.selectionStart, anArea.selectionEnd - anArea.selectionStart);
		text[2] = anArea.value.substr(anArea.selectionEnd);
		if (text[1]=='') { if (text[0].lastIndexOf(startTxt) > text[0].lastIndexOf(endTxt)) startTxt=midTxt=''; else endTxt=''; }
		var caretPos = anArea.selectionEnd + startTxt.length + midTxt.length + endTxt.length;
		anArea.value = text[0] + startTxt + midTxt + text[1] + endTxt + text[2];
		window.status += text[1];
		anArea.focus();
		if (anArea.setSelectionRange) {
			anArea.setSelectionRange(caretPos, caretPos);
			}
		}
	else {
		anArea.value += startTxt + midTxt + endTxt;
		}
	window.status += endTxt;
	}

function getSelectionIn(anArea) {
	if (typeof(anArea.selectionStart) != "undefined") 
		return anArea.value.substr(anArea.selectionStart, anArea.selectionEnd - anArea.selectionStart)
	if (document.selection) 
		return ( anArea.caretPos ? anArea.caretPos.text : document.selection.createRange().text );
	return "";
	}

function doTagAttrib(anArea,aTag,anAttrib,aValue) {
	if (aValue || anAttrib) doStartEnd(anArea,'<'+aTag, ' '+anAttrib+'=\"'+aValue+'\">','</'+aTag+'>'); else doStartEnd(anArea,'<'+aTag,'>', '</'+aTag+'>');
	}

function doTag(anArea,aTag) {
	doStartEnd(anArea,'<'+aTag,'>', '</'+aTag+'>');
	}

function doLink(anArea) { 
	var text = getSelectionIn(anArea);
	var isUrl = new RegExp(/(((ht|f)tp(s?))\:\/\/|www.)([^/: ]+)(:\d*)?([^# ]*)/);
	if (text.length>0 && !isUrl.test(text)) text=prompt('Enter url','http://www.'+(text.indexOf(' ')<1?text:''))
	if (text && text.substr(0,4)=='www.') text='http://'+text;
	doTagAttrib(anArea,'a','href',text);
	}

function storeCaret(anArea) {
	if (anArea.createTextRange) anArea.caretPos = document.selection.createRange().duplicate();
	}

function doKey(anEvent) {
	if(!anEvent && event) anEvent=event; //case IE doesn't pass the event.
	if (anEvent) { //an some people dont pass it or make it global.
//		window.status=anEvent+' '+anEvent.keyCode+' '+anEvent.ctrlKey;
		if (anEvent.ctrlKey) { 
			if (anEvent.keyCode==73 || anEvent.keyCode==66 || anEvent.keyCode==85 || anEvent.keyCode==80) { 	
// b, i, u or p
				doTagAttrib(this,String.fromCharCode(anEvent.keyCode+32),'',''); 
				return false;
				}
			if (anEvent.keyCode==76) { 
				doLink(this); 
				return false
				}
			}
		}
	return true;
	}
	
function doButton(aTitle, aScript, aLable) {
	document.write('<a style="cursor:hand;background-color:silver;border-style:outset;border-width:thin"');
	document.write(' title=\''+aTitle+'\'');
	document.write(' onClick="javascript:'+aScript+';"');
	document.write('>'+aLable+'<\/A> ');
	}

function preview(anAreaID, form, who) {
var anArea = AJS.getElement(anAreaID);
var start=end=ref=txt="";
var mode = form.act.selectedIndex;
var act = form.act.options[mode].value;
var link = form.file.value;
var text = form.txt.value;
var privacy = form.private.selectedIndex;
	if (anArea) {
		anArea.innerHTML = "";
		if (act.charAt(act.length-1)==":") {
			anArea.innerHTML += act;
			start += "<ul><li>";
			end = "</ul>"+end;
			}
		else { 
			ref = act+": ";
			}
		if (mode==3) {
			ref = " shares";
			text = "<pre>"+text+"</pre>"
			}
		if (mode==2) {
			ref = " refers to: ";
			}
		if (mode==0) {
			ref = " asks: ";
			}
		if (privacy!=1) {
			start += "<a href=\"/techref/member/" + who + "/index.htm\">" + who + "</a> " + ref
			if (text.length > 150) { start += "<blockquote>"; end = "</blockquote>" + end; }
			if (text.length > 1) { start += "&quot;"; end = "&quot;" + end; }
			}
		if (link.length>1) {
			if (form.file.urlstatus=='bad') start += "<a title=\"ERROR: Address Not found\" style=\"background:red;\" href=\""+link+"\">"; else start += "<a href=\""+link+"\">";
			if (act.charAt(act.length-1)==":") start += link + "</a> "; else end = "</a>"+end;
			}
		anArea.innerHTML += start + text + end;
		} 
	}
	
function checkLink(field) {
var me = this;
	if (field.value.substr(0,4)=="www.") field.value = "http://"+field.value;
	if (field.value.substr(0,7)!="http://") return;
	me.url = field.value;
	me.field = field;
	try {
		var d = AJS.loadJSONDoc(field.value);
		var reqfailed = function(req) {
			me.field.style.backgroundImage="url(/images/bad.gif)";
			window.status = me.url+" is not a valid address.";
			me.field.urlstatus="bad";
			};
		me.field.style.backgroundImage="url(/images/good.gif)";	//assume it's good.
		d.addErrback(reqfailed);	//unless it aint
		d.sendReq();
		}
	catch (e) {};
	}


function dictword() {
	if (document.getSelection) {
		t = document.getSelection();
		opennewdictwin(t);
		}
	else {
		t = document.selection.createRange();
		if(document.selection.type == 'Text' && t.text != '') {
			document.selection.empty();
			opennewdictwin(t.text);
			}
		}
	}

function opennewdictwin(text) {
	while (text.substr(text.length-1,1)==' ') 
		text=text.substr(0,text.length-1)
	while (text.substr(0,1)==' ') 
		text=text.substr(1)
	if (text > '') {
		document.location='http://golovchenko.org/cgi-bin/wnsearch?q='+escape(text);
		}
	}
