function messageBox(mbWidth, mbHeight, mbMessage, mbTitle, mbButtonCaptions, mbBtnCode, mbCallbackArgs)
{
	Engine.PushGuiPage(
		"page_msgbox.xml",
		{
			"width": mbWidth,
			"height": mbHeight,
			"message": mbMessage,
			"title": mbTitle,
			"buttonCaptions": mbButtonCaptions
		},
		btnCode => {
			if (mbBtnCode !== undefined && mbBtnCode[btnCode])
				mbBtnCode[btnCode](mbCallbackArgs ? mbCallbackArgs[btnCode] : undefined);
		});
}

function timedConfirmation(width, height, message, timeParameter, timeout, title, buttonCaptions, btnCode, callbackArgs)
{
	Engine.PushGuiPage(
		"page_timedconfirmation.xml",
		{
			"width": width,
			"height": height,
			"message": message,
			"timeParameter": timeParameter,
			"timeout": timeout,
			"title": title,
			"buttonCaptions": buttonCaptions
		},
		button => {
			if (btnCode !== undefined && btnCode[button])
				btnCode[button](callbackArgs ? callbackArgs[button] : undefined);
		});
}

function colorMixer(color, callback)
{
	Engine.PushGuiPage(
		"page_colormixer.xml",
		color,
		result => {
			callback(result);
		}
	);
}

function openURL(url)
{
	Engine.OpenURL(url);

	messageBox(
		600, 200,
		sprintf(
			translate("Opening %(url)s\n in default web browser. Please wait…"),
			{ "url": url }
		),
		translate("Opening page"));
}
