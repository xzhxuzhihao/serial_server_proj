// The ErrorHandler class is used to log one or more error
// messages encountered when validating form fields. After
// all errors have been logged the errors can then be displayed
// in an alert box, the failed form field controls highlighted, and
// focus/selection set on the first failingform field control.
//
// NOTE: the ErrorHandler object should be instantiated at
// global scope. It must be kept around for subsequent
// calls. Use the clearErrors message between those calls to
// reset the highlighted fields on the screen.
//
// EXAMPLE:
// var err = new ErrorHandler();
// function doValidate() {
// err.clearErrors();
// if (!isValidIP(document.network_form.ip_addr.value))
// err.addError(document.network_form.ip_addr, "Invalid IP address.");
// if (!isDottedForm(document.network_form.submask.value))
// err.addError(document.network_form.submask, "Invalid subnet mask.");
// ...
// err.showError();
// return !err.hasError();
// }
// Constructor
function ErrorHandler() {
	this.errors = new Array();
	this.errorCtrls = new Array();
	this.warnings = new Array();
	this.warningCtrls = new Array();
}
// Add an error message and the form control that caused the error.
ErrorHandler.prototype.addError = function(formCtrl, errMsg) {
	if (errMsg != null) {
		this.errors[this.errors.length] = errMsg;
	}
	if (formCtrl != null) {
		this.errorCtrls[this.errorCtrls.length] = formCtrl;
	}
}
// Determine if errors have been logged (returns boolean).
ErrorHandler.prototype.hasError = function() {
	return (this.errors.length > 0);
}
// Show error messages and process form controls (highlight and set focus).
ErrorHandler.prototype.showError = function() {
	if (this.errors.length == 0) {
		return true;
	} else if (this.errors.length == 1) {
		alert('Please correct the following problem and try again:\n\n'
				+ this.errors[0]);
	} else {
		var msg = 'Please correct the following problems and try again:\n';
		var i;
		for (i = 0; i < this.errors.length; ++i) {
			msg = msg + "\n- " + this.errors[i];
		}
		alert(msg);
	}
	var f;
	var focusSet = false;
	for (f = 0; f < this.errorCtrls.length; ++f) {
		// Older browsers don't support the className attribute.
		if (this.errorCtrls[f].className != null)
			this.errorCtrls[f].className = 'field-error';
		// Set focus and select text on first failed field.
		if (!focusSet) {
			this.errorCtrls[f].focus();
			if (this.errorCtrls[f].select)
				this.errorCtrls[f].select();
			focusSet = true;
		}
	}
	return false;
}
// Clear all error messages and reset the styles on the form controls that were
// previously flagged for errors.
ErrorHandler.prototype.clearErrors = function() {
	// Reset messages
	this.errors.length = 0;
	// Reset styles for failed controls
	for (f = 0; f < this.errorCtrls.length; ++f) {
		// Older browsers don't support the className attribute.
		if (this.errorCtrls[f].className != null)
			this.errorCtrls[f].className = '';
	}
	this.errorCtrls.length = 0;
}
// Add a warning message and the form control that caused the warning.
ErrorHandler.prototype.addWarning = function(formCtrl, warningMsg) {
	if (warningMsg != null) {
		this.warnings[this.warnings.length] = warningMsg;
	}
	if (formCtrl != null) {
		this.warningCtrls[this.warningCtrls.length] = formCtrl;
	}
}
// Determine if warnings have been logged (returns boolean).
ErrorHandler.prototype.hasWarning = function() {
	return (this.warnings.length > 0);
}
// Show warning messages and process form controls (highlight and set focus).
ErrorHandler.prototype.showWarning = function() {
	var result = false;
	if (this.warnings.length == 0) {
		return true;
	} else if (this.warnings.length == 1) {
		result = confirm('Please verify the following problem:\n\n'
				+ this.warnings[0]
				+ '\n\nClick OK to continue saving the settings.');
	} else {
		var msg = 'Please verify the following problems:\n';
		var i;
		for (i = 0; i < this.warnings.length; ++i) {
			msg = msg + "\n- " + this.warnings[i];
		}
		msg = msg + '\n\nClick OK to continue saving the settings.';
		result = confirm(msg);
	}
	if (result)
		return true;
	var f;
	var focusSet = false;
	for (f = 0; f < this.warningCtrls.length; ++f) {
		// Older browsers don't support the className attribute.
		if (this.warningCtrls[f].className != null)
			this.warningCtrls[f].className = 'field-warning';
		// Set focus and select text on first failed field.
		if (!focusSet) {
			this.warningCtrls[f].focus();
			if (this.warningCtrls[f].select)
				this.warningCtrls[f].select();
			focusSet = true;
		}
	}
	return false;
}
// Clear all warning messages and reset the styles on the form controls that
// were
// previously flagged for warnings.
ErrorHandler.prototype.clearWarnings = function() {
	// Reset messages
	this.warnings.length = 0;
	// Reset styles for failed controls
	for (f = 0; f < this.warningCtrls.length; ++f) {
		// Older browsers don't support the className attribute.
		if (this.warningCtrls[f].className != null)
			this.warningCtrls[f].className = '';
	}
	this.warningCtrls.length = 0;
}
// Determine if errors/warnings have been logged (returns boolean).
ErrorHandler.prototype.hasMessage = function() {
	return (this.hasError() || this.hasWarning());
}
// Clear all error/warning messages and reset the styles on the form controls
// that were
// previously flagged for errors/warnings.
ErrorHandler.prototype.clearMessages = function() {
	this.clearErrors();
	this.clearWarnings();
}
// Show error/warning messages and process form controls (highlight and set
// focus).
ErrorHandler.prototype.showMessage = function() {
	if (this.hasError())
		return this.showError();
	if (this.hasWarning())
		return this.showWarning();
	return true;
}
