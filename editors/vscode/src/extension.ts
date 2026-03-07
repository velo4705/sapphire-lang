import * as path from 'path';
import { workspace, ExtensionContext, debug, DebugConfiguration, WorkspaceFolder, CancellationToken, ProviderResult } from 'vscode';
import {
    LanguageClient,
    LanguageClientOptions,
    ServerOptions,
    TransportKind
} from 'vscode-languageclient/node';

let client: LanguageClient;

export function activate(context: ExtensionContext) {
    console.log('Sapphire extension is now active!');

    // Get LSP configuration
    const config = workspace.getConfiguration('sapphire');
    const lspEnabled = config.get<boolean>('lsp.enabled', true);
    
    if (lspEnabled) {
        // Path to LSP server executable
        const lspPath = config.get<string>('lsp.path', 'sapphire-lsp');
        
        // Server options
        const serverOptions: ServerOptions = {
            command: lspPath,
            args: [],
            transport: TransportKind.stdio
        };

        // Client options
        const clientOptions: LanguageClientOptions = {
            documentSelector: [{ scheme: 'file', language: 'sapphire' }],
            synchronize: {
                fileEvents: workspace.createFileSystemWatcher('**/.spp')
            }
        };

        // Create the language client
        client = new LanguageClient(
            'sapphireLSP',
            'Sapphire Language Server',
            serverOptions,
            clientOptions
        );

        // Start the client (and server)
        client.start();
        
        console.log('Sapphire LSP client started');
    }

    // Register debug adapter
    const debugProvider = new SapphireDebugConfigurationProvider();
    context.subscriptions.push(
        debug.registerDebugConfigurationProvider('sapphire', debugProvider)
    );

    console.log('Sapphire debug adapter registered');
}

export function deactivate(): Thenable<void> | undefined {
    if (!client) {
        return undefined;
    }
    return client.stop();
}

/**
 * Debug configuration provider for Sapphire
 */
class SapphireDebugConfigurationProvider {
    resolveDebugConfiguration(
        folder: WorkspaceFolder | undefined,
        config: DebugConfiguration,
        token?: CancellationToken
    ): ProviderResult<DebugConfiguration> {
        // If launch.json is missing or empty
        if (!config.type && !config.request && !config.name) {
            const editor = workspace.textDocuments.find(doc => doc.languageId === 'sapphire');
            if (editor) {
                config.type = 'sapphire';
                config.name = 'Launch';
                config.request = 'launch';
                config.program = editor.fileName;
                config.stopOnEntry = false;
            }
        }

        if (!config.program) {
            return undefined;
        }

        return config;
    }
}

