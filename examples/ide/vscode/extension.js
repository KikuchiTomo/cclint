// VSCode Extension for cclint LSP integration
// This file demonstrates how to integrate cclint-lsp with VSCode

const vscode = require('vscode');
const { LanguageClient, TransportKind } = require('vscode-languageclient/node');

let client;

function activate(context) {
    console.log('cclint extension is now active');

    // Get configuration
    const config = vscode.workspace.getConfiguration('cclint');
    const lspEnabled = config.get('lsp.enabled', false);

    if (!lspEnabled) {
        console.log('cclint LSP is disabled');
        return;
    }

    const serverPath = config.get('lsp.path', 'cclint-lsp');
    const configPath = config.get('lsp.configPath', '');

    // LSP server options
    const serverOptions = {
        run: {
            command: serverPath,
            transport: TransportKind.stdio
        },
        debug: {
            command: serverPath,
            transport: TransportKind.stdio,
            options: {
                env: { ...process.env, CCLINT_DEBUG: '1' }
            }
        }
    };

    // LSP client options
    const clientOptions = {
        documentSelector: [
            { scheme: 'file', language: 'cpp' },
            { scheme: 'file', language: 'c' }
        ],
        synchronize: {
            fileEvents: vscode.workspace.createFileSystemWatcher('**/*.{cpp,hpp,c,h}')
        },
        initializationOptions: {
            configPath: configPath
        }
    };

    // Create language client
    client = new LanguageClient(
        'cclintLanguageServer',
        'cclint Language Server',
        serverOptions,
        clientOptions
    );

    // Start the client
    client.start();

    // Register commands
    context.subscriptions.push(
        vscode.commands.registerCommand('cclint.restart', () => {
            if (client) {
                client.stop();
                client.start();
                vscode.window.showInformationMessage('cclint LSP server restarted');
            }
        })
    );

    context.subscriptions.push(
        vscode.commands.registerCommand('cclint.runOnCurrentFile', () => {
            const editor = vscode.window.activeTextEditor;
            if (!editor) {
                return;
            }

            const document = editor.document;
            const cclintPath = config.get('path', 'cclint');
            const configFile = config.get('configFile', '');
            const outputFormat = config.get('outputFormat', 'text');

            const terminal = vscode.window.createTerminal('cclint');
            let command = `${cclintPath}`;
            if (configFile) {
                command += ` --config=${configFile}`;
            }
            command += ` --format=${outputFormat}`;
            command += ` g++ ${document.fileName}`;

            terminal.sendText(command);
            terminal.show();
        })
    );
}

function deactivate() {
    if (client) {
        return client.stop();
    }
}

module.exports = {
    activate,
    deactivate
};
