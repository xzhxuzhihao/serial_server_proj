//
// Validation functions
//
// These validation functions are intended primarily for the validation of user
// input in form controls. Most work with string values.
//

function isDottedForm(strVal) {
	if ((strVal == null) || (strVal.length < 7))
		return false;
	var rc = true;
	if (strVal.search(/^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/) != -1) {
		var myArray = strVal.split(/\./);
		if (myArray[0] > 255 || myArray[1] > 255 || myArray[2] > 255
				|| myArray[3] > 255) {
			rc = false;
		}
	} else {
		rc = false;
	}
	return rc;
}
function isValidNetworkName(strVal) {
	if (strVal == null)
		return false;
	if (isEmptyString(strVal))
		return false;
	if (strVal.search(/^[a-zA-Z0-9.-]*$/) != -1)
		return true;
	return false;
}
function isValidGateway(strVal) {
	if ((strVal == null) || (strVal.length < 7))
		return false;
	var rc = true;
	if (isDottedForm(strVal)) {
		if (strVal.search(/^0{1,3}\.0{1,3}\.0{1,3}\.0{1,3}$/) != -1)
			rc = false;
	} else {
		rc = false;
	}
	return rc;
}
function isValidIP(strVal) {
	if ((strVal == null) || (strVal.length < 7))
		return false;
	var rc = true;
	if (isDottedForm(strVal)) {
		var myArray = strVal.split(/\./);
		if (myArray[0] == 255 && myArray[1] == 255 && myArray[2] == 255
				&& myArray[3] == 255) {
			rc = false;
		}
	} else {
		rc = false;
	}
	return rc;
}
function isValidIpPrefix(strVal) {
	if ((strVal == null) || (strVal.length < 5))
		return false;
	var temp = strVal + ".253";
	var rc = false;
	rc = isValidIP(temp);
	return rc;
}
function isValidNonZeroIP(strVal) {
	if ((strVal == null) || (strVal.length < 7))
		return false;
	var rc = true;
	if (isDottedForm(strVal)) {
		var myArray = strVal.split(/\./);
		if (myArray[0] == 255 && myArray[1] == 255 && myArray[2] == 255
				&& myArray[3] == 255)
			rc = false;
		else if (myArray[0] == 0 && myArray[1] == 0 && myArray[2] == 0
				&& myArray[3] == 0)
			rc = false;
	} else
		rc = false;
	return rc;
}
function isNonZeroDottedForm(strVal) {
	if ((strVal == null) || (strVal.length < 7))
		return false;
	var rc = true;
	if (isDottedForm(strVal)) {
		var myArray = strVal.split(/\./);
		if (myArray[0] == 0 && myArray[1] == 0 && myArray[2] == 0
				&& myArray[3] == 0) {
			rc = false;
		}
	} else {
		rc = false;
	}
	return rc;
}
function isValidMACAddress(strVal) {
	if (strVal == null || (strVal.length != 17))
		return false;
	var myArray = strVal.split(/:/);
	if (myArray.length != 6)
		return false;
	for (i = 0; i < myArray.length; i++) {
		if (myArray[i].search(/^[a-fA-F0-9][a-fA-F0-9]$/) == -1)
			return false;
	}
	return true;
}
function isBroadcastMACAddress(strVal) {
	if ((strVal == null) || (strVal.length < 17))
		return false;
	var rc = true;
	if (isValidMACAddress(strVal)) {
		var myArray = strVal.split(/:/);
		for (i = 0; i < myArray.length; i++) {
			if (myArray[i].search(/^[fF][fF]$/) == -1) {
				return false;
			}
		}
	} else {
		rc = false;
	}
	return rc;
}
function isZeroMACAddress(strVal) {
	if ((strVal == null) || (strVal.length < 17))
		return false;
	var rc = true;
	if (isValidMACAddress(strVal)) {
		var myArray = strVal.split(/:/);
		for (i = 0; i < myArray.length; i++) {
			if (myArray[i] != 0) {
				return false;
			}
		}
	} else {
		rc = false;
	}
	return rc;
}
function isValidEmail(strVal) {
	if (strVal == null || (strVal.length < 5))
		return false;
	return (strVal
			.search(/^[a-zA-Z0-9-_.]*[a-zA-Z0-9-_.]\@[a-zA-Z0-9].+[a-zA-Z0-9]+[a-zA-Z0-9]$/) != -1);
}
function isValidInteger(strVal) {
	if ((strVal == null) || (strVal.length == 0))
		return false;
	var intFound = false;
	var i;
	for (i = 0; i < strVal.length; i++) {
		// Check that current character is number.
		var c = strVal.charAt(i);
		if (c == "-") {
			// minus sign must be come before any numbers
			if (intFound) {
				return false;
			}
		} else if (isDigit(c)) {
			intFound = true;
		} else {
			return false;
		}
	}
	return intFound;
}
function isValidHexadecimal(strVal) {
	if ((strVal == null) || (strVal.length == 0))
		return false;
	var hexVal = strVal.toUpperCase();
	var i;
	for (i = 0; i < hexVal.length; i++) {
		// Check that current character is number.
		var c = hexVal.charAt(i);
		if (!isHexDigit(c)) {
			return false;
		}
	}
	return true;
}
function isInRange(strVal, minInt, maxInt) {
	if ((strVal == null) || (strVal.length == 0))
		return false;
	if (!isValidInteger(strVal))
		return false;
	if ((minInt == null) || (maxInt == null))
		return false;
	var val = parseInt(strVal);
	return ((val >= minInt) && (val <= maxInt));
}
function isValidSigned8(strVal) {
	return isInRange(strVal, -128, 127);
}
function isValidUnsigned8(strVal) {
	return isInRange(strVal, 0, 255);
}
function isValidSigned16(strVal) {
	return isInRange(strVal, -32768, 32767);
}
function isValidUnsigned16(strVal) {
	return isInRange(strVal, 0, 65535);
}
function isValidSigned32(strVal) {
	return isInRange(strVal, -2147483648, 2147483647);
}
function isValidUnsigned32(strVal) {
	return isInRange(strVal, 0, 4294967295);
}
function isValidPassword(strVal) {
	if (strVal == null || strVal.length < 1 || strVal.length > 15)
		return false;
	return true;
}
function isValidUserPassword(strVal) {
	if (strVal == null || strVal.length < 4 || strVal.length > 16)
		return false;
	return true;
}
function isEmptyString(strVal, trimFirst) {
	if ((strVal == null))
		return true;
	if ((trimFirst == null) || (trimFirst == true))
		strVal = trim(strVal);
	return (strVal.length == 0);
}
function trim(strVal) {
	if ((strVal == null) || (strVal.length == 0))
		return "";
	// trim leading blanks
	var i = 0;
	while ((i < strVal.length) && (strVal.charAt(i) == " ")) {
		++i;
	}
	var tmpVal = strVal.substring(i);
	// trim trailing blanks
	i = tmpVal.length;
	while ((i > 0) && (tmpVal.charAt(i - 1) == " ")) {
		--i;
	}
	return tmpVal.substring(0, i);
}
function isDigit(c) {
	return ((c >= "0") && (c <= "9"))
}
function isHexDigit(c) {
	return (((c >= "0") && (c <= "9")) || ((c >= "A") && (c <= "F")))
}
function isValidIpv6Address(strVal) {
	if (strVal == null || (strVal.length < 3))
		return false;
	var myArray = strVal.split(/:/);
	if (myArray.length < 1)
		return false;
	for (i = 0; i < myArray.length; i++) {
		if (isValidHexadecimal(myArray[i]) == false)
			return false;
	}
	return true;
}
function isValidNonZeroIPv6(strVal) {
	if (strVal == null || (strVal.length < 3))
		return false;
	var myArray = strVal.split(/:/);
	if (myArray.length < 1)
		return false;
	for (i = 0; i < myArray.length; i++) {
		if (myArray[i] != 0)
			return true;
	}
	return false;
}
