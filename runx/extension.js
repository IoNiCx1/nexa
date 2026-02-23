const vscode = require('vscode');

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

        const filePath = editor.document.fileName;

        const terminal = vscode.window.createTerminal("RunX");
        terminal.show();
        terminal.sendText(`nexa "${filePath}"`);
    });

    context.subscriptions.push(disposable);
}

function deactivate() {}

module.exports = {
    activate,
    deactivate
};