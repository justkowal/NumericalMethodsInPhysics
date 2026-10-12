const vscode = require('vscode');
const path = require('path');

/**
 * Normalizes symbol names by removing parameter lists and template arguments,
 * while preserving operator signatures like operator(), operator[], etc.
 */
function cleanSymbolName(rawName) {
    if (!rawName) return '';
    let name = rawName.trim();

    // Check if it is an operator overload
    const opMatch = name.match(/operator\s*(\(\)|\[\]|[+\-*/%=!<>^&|~]+|[a-zA-Z_0-9]+)/);
    if (opMatch) {
        return `operator${opMatch[1]}`;
    }

    // Strip return type prefix if present (e.g. "void func" -> "func")
    const words = name.split(/\s+/);
    if (words.length > 1 && !name.includes('(')) {
        name = words[words.length - 1];
    }

    // Strip parameter lists: "myFunc(int a, double b)" -> "myFunc"
    const parenIdx = name.indexOf('(');
    if (parenIdx !== -1) {
        name = name.substring(0, parenIdx).trim();
    }

    // Strip template parameters: "MyClass<T>" -> "MyClass"
    const angleIdx = name.indexOf('<');
    if (angleIdx !== -1) {
        name = name.substring(0, angleIdx).trim();
    }

    // Remove any leftover pointer/reference symbols
    name = name.replace(/^[*&]+/, '').trim();
    return name;
}

/**
 * Traverses a DocumentSymbol tree to locate the chain of symbols enclosing the position.
 */
function findEnclosingSymbols(symbols, position) {
    let result = [];
    for (const sym of symbols) {
        if (sym.range.contains(position)) {
            result.push(sym);
            if (sym.children && sym.children.length > 0) {
                const inner = findEnclosingSymbols(sym.children, position);
                result = result.concat(inner);
            }
            break;
        }
    }
    return result;
}

/**
 * Lightweight regex-based fallback to extract enclosing symbol if LSP is not yet active.
 */
function fallbackSymbolExtract(document, position) {
    const text = document.getText();
    const posOffset = document.offsetAt(position);
    const beforeText = text.substring(0, posOffset);
    const lang = document.languageId;

    if (lang === 'python') {
        const lines = beforeText.split('\n');
        let funcName = null;
        let className = null;
        for (let i = lines.length - 1; i >= 0; i--) {
            const line = lines[i];
            const defMatch = line.match(/^\s*def\s+([A-Za-z0-9_]+)\s*\(/);
            if (!funcName && defMatch) {
                funcName = defMatch[1];
            }
            const classMatch = line.match(/^\s*class\s+([A-Za-z0-9_]+)/);
            if (classMatch) {
                className = classMatch[1];
                break;
            }
        }
        if (className && funcName) return `${className}.${funcName}`;
        if (funcName) return funcName;
        if (className) return className;
    } else {
        // C/C++ fallback
        const lines = beforeText.split('\n');
        let memberName = null;
        let className = null;
        for (let i = lines.length - 1; i >= 0; i--) {
            const line = lines[i];
            // Out of line definition: Matrix::rows or Matrix::Matrix
            const outOfLineMatch = line.match(/\b([A-Za-z0-9_]+)::([A-Za-z0-9_]+|operator\s*(\(\)|\[\]|[+\-*/%=!<>]+))\s*\(/);
            if (outOfLineMatch) {
                return `${outOfLineMatch[1]}::${cleanSymbolName(outOfLineMatch[2])}`;
            }
            // Class declaration
            const classMatch = line.match(/\b(class|struct)\s+([A-Za-z0-9_]+)\b/);
            if (!className && classMatch) {
                className = classMatch[2];
            }
            // Method or function definition/declaration
            const funcMatch = line.match(/\b([A-Za-z0-9_]+|operator\s*(\(\)|\[\]|[+\-*/%=!<>]+))\s*\([^)]*\)\s*(?:const)?\s*(?:noexcept)?\s*[{;]?$/);
            if (!memberName && funcMatch) {
                memberName = cleanSymbolName(funcMatch[1]);
            }
        }
        if (className && memberName) return `${className}::${memberName}`;
        if (memberName) return memberName;
        if (className) return className;
    }

    // Word at cursor fallback
    const wordRange = document.getWordRangeAtPosition(position);
    if (wordRange) {
        return document.getText(wordRange);
    }
    return null;
}

/**
 * Resolves the semantic query string for the active cursor or selection.
 */
async function resolveSemanticQuery(editor) {
    const document = editor.document;
    const selection = editor.selection;

    // 1. If multiple lines are selected, return explicit line range
    if (!selection.isEmpty && selection.start.line !== selection.end.line) {
        const startLine = selection.start.line + 1;
        const endLine = selection.end.line + 1;
        return {
            query: `${startLine}-${endLine}`,
            isLineRange: true,
            startLine,
            endLine,
            symbolName: `Lines ${startLine}-${endLine}`
        };
    }

    const position = selection.active;
    const lang = document.languageId;
    let symbolChain = [];

    try {
        const symbols = await vscode.commands.executeCommand(
            'vscode.executeDocumentSymbolProvider',
            document.uri
        );
        if (symbols && symbols.length > 0) {
            symbolChain = findEnclosingSymbols(symbols, position);
        }
    } catch (err) {
        // Document symbol provider failed or unsupported
    }

    if (symbolChain.length > 0) {
        // Filter relevant symbols (classes, structs, functions, methods, constructors)
        const relevantSymbols = symbolChain.filter(sym => {
            const k = sym.kind;
            return (
                k === vscode.SymbolKind.Class ||
                k === vscode.SymbolKind.Struct ||
                k === vscode.SymbolKind.Interface ||
                k === vscode.SymbolKind.Function ||
                k === vscode.SymbolKind.Method ||
                k === vscode.SymbolKind.Constructor ||
                k === vscode.SymbolKind.Namespace
            );
        });

        const activeSym = relevantSymbols.length > 0
            ? relevantSymbols[relevantSymbols.length - 1]
            : symbolChain[symbolChain.length - 1];

        const startLine = activeSym.range.start.line + 1;
        const endLine = activeSym.range.end.line + 1;

        if (lang === 'python') {
            const classSym = relevantSymbols.find(s => s.kind === vscode.SymbolKind.Class);
            const methodSym = relevantSymbols.find(s => s.kind === vscode.SymbolKind.Method || s.kind === vscode.SymbolKind.Function);

            if (classSym && methodSym && classSym !== methodSym) {
                const q = `${cleanSymbolName(classSym.name)}.${cleanSymbolName(methodSym.name)}`;
                return { query: q, isLineRange: false, startLine, endLine, symbolName: q };
            } else {
                const q = cleanSymbolName(activeSym.name);
                return { query: q, isLineRange: false, startLine, endLine, symbolName: q };
            }
        } else {
            // C/C++ and others
            const classSym = relevantSymbols.find(s => s.kind === vscode.SymbolKind.Class || s.kind === vscode.SymbolKind.Struct);
            const methodSym = relevantSymbols.find(s =>
                s.kind === vscode.SymbolKind.Method ||
                s.kind === vscode.SymbolKind.Function ||
                s.kind === vscode.SymbolKind.Constructor
            );

            if (classSym && methodSym && classSym !== methodSym) {
                const cName = cleanSymbolName(classSym.name);
                let mName = cleanSymbolName(methodSym.name);
                // Check if constructor has same name or method starts with class
                if (mName.startsWith(cName + '::')) {
                    return { query: mName, isLineRange: false, startLine, endLine, symbolName: mName };
                }
                const q = `${cName}::${mName}`;
                return { query: q, isLineRange: false, startLine, endLine, symbolName: q };
            } else {
                const q = cleanSymbolName(activeSym.name);
                return { query: q, isLineRange: false, startLine, endLine, symbolName: q };
            }
        }
    }

    // 2. Fallback text parsing
    const fallback = fallbackSymbolExtract(document, position);
    if (fallback) {
        return {
            query: fallback,
            isLineRange: false,
            startLine: position.line + 1,
            endLine: position.line + 1,
            symbolName: fallback
        };
    }

    // 3. Fallback to current line range
    const curLine = position.line + 1;
    return {
        query: `${curLine}-${curLine}`,
        isLineRange: true,
        startLine: curLine,
        endLine: curLine,
        symbolName: `Line ${curLine}`
    };
}

/**
 * Calculates optimal relative path for LaTeX \codesnippet macro.
 * If file is in ProjectXX/src/Matrix.cpp, returns src/Matrix.cpp (ideal for root=..)
 */
function resolveSnippetFilePath(document) {
    const filePath = document.uri.fsPath;
    const workspaceFolder = vscode.workspace.getWorkspaceFolder(document.uri);
    const workspaceRoot = workspaceFolder ? workspaceFolder.uri.fsPath : path.dirname(filePath);

    // Check if the file is inside a Project directory (e.g. Project01)
    const projectMatch = filePath.match(/(.*?[\/\\]Project\d{2})[\/\\](.*)/);
    if (projectMatch) {
        // Return relative to ProjectXX root (e.g. src/Matrix.cpp, include/Matrix.hpp)
        return projectMatch[2].replace(/\\/g, '/');
    }

    // Otherwise relative to workspace root
    const rel = path.relative(workspaceRoot, filePath).replace(/\\/g, '/');
    return rel;
}

/**
 * Creates clean LaTeX label identifier from symbol name
 */
function makeLabelKey(symbolQuery) {
    return symbolQuery
        .replace(/[:.]+/g, '_')
        .replace(/[^A-Za-z0-9_-]/g, '')
        .toLowerCase();
}

function activate(context) {
    // 1. Copy Semantic Symbol Path
    const copySemanticPathCmd = vscode.commands.registerCommand('codesnippet.copySemanticPath', async () => {
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            vscode.window.showWarningMessage('No active code editor.');
            return;
        }

        const info = await resolveSemanticQuery(editor);
        await vscode.env.clipboard.writeText(info.query);
        vscode.window.setStatusBarMessage(`CodeSnippet: Copied "${info.query}" to clipboard`, 3000);
    });

    // 2. Copy LaTeX \codesnippet Macro
    const copyLatexMacroCmd = vscode.commands.registerCommand('codesnippet.copyLatexMacro', async () => {
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            vscode.window.showWarningMessage('No active code editor.');
            return;
        }

        const info = await resolveSemanticQuery(editor);
        const relPath = resolveSnippetFilePath(editor.document);
        const macro = `\\codesnippet{${relPath}}{${info.query}}`;

        await vscode.env.clipboard.writeText(macro);
        vscode.window.setStatusBarMessage(`CodeSnippet: Copied "${macro}" to clipboard`, 3000);
    });

    // 3. Copy Detailed \codesnippet Macro with Title & Label
    const copyLatexMacroDetailedCmd = vscode.commands.registerCommand('codesnippet.copyLatexMacroDetailed', async () => {
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            vscode.window.showWarningMessage('No active code editor.');
            return;
        }

        const info = await resolveSemanticQuery(editor);
        const relPath = resolveSnippetFilePath(editor.document);
        const labelKey = makeLabelKey(info.query);
        const displayTitle = info.symbolName || info.query;

        const macro = `\\codesnippet[
    title = {${displayTitle}},
    label = snip:${labelKey}
]{${relPath}}{${info.query}}`;

        await vscode.env.clipboard.writeText(macro);
        vscode.window.setStatusBarMessage(`CodeSnippet: Copied detailed snippet macro to clipboard`, 3000);
    });

    // 4. Copy Real Location (file:lines)
    const copyLocationCmd = vscode.commands.registerCommand('codesnippet.copyLocation', async () => {
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            vscode.window.showWarningMessage('No active code editor.');
            return;
        }

        const info = await resolveSemanticQuery(editor);
        const relPath = resolveSnippetFilePath(editor.document);
        const loc = `${relPath}:${info.startLine}-${info.endLine}`;

        await vscode.env.clipboard.writeText(loc);
        vscode.window.setStatusBarMessage(`CodeSnippet: Copied "${loc}" to clipboard`, 3000);
    });

    context.subscriptions.push(
        copySemanticPathCmd,
        copyLatexMacroCmd,
        copyLatexMacroDetailedCmd,
        copyLocationCmd
    );
}

function deactivate() {}

module.exports = {
    activate,
    deactivate
};
