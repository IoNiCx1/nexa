const vscode = require('vscode');
const os = require('os');
const path = require('path');

function getCompilerPath(context) {
    const platform = os.platform();

    if (platform === "win32") {
        return path.join(context.extensionPath, "bin", "windows", "nexa.exe");
    } 
    else if (platform === "linux") {
        return path.join(context.extensionPath, "bin", "linux", "nexa");
    } 
    else if (platform === "darwin") {
        return path.join(context.extensionPath, "bin", "mac", "nexa");
    } 
    else {
        return null;
    }
}

function activate(context) {

    let disposable = vscode.commands.registerCommand('runx.runFile', function () {

        const editor = vscode.window.activeTextEditor;

        if (!editor) {
            vscode.window.showErrorMessage("No active file open.");
            return;
        }

        if (editor.document.languageId !== "nexa") {
            vscode.window.showErrorMessage("Not a Nexa (.nx) file.");
            return;
        }

        const compilerPath = getCompilerPath(context);

        if (!compilerPath) {
            vscode.window.showErrorMessage("Unsupported OS.");
            return;
        }

        const filePath = editor.document.fileName;

        const terminal = vscode.window.createTerminal("RunX");
        terminal.show();
        terminal.sendText(`echo RUNNING: "${compilerPath}" "${filePath}"`);
terminal.sendText(`"${compilerPath}" "${filePath}"`);
    });

    context.subscriptions.push(disposable);
}

function deactivate() {}

module.exports = { activate, deactivate };