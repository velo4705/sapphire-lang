;;; sapphire-mode.el --- Major mode for the Sapphire programming language

;; Author: Sapphire Language Team
;; Version: 1.0-beta.8
;; Keywords: languages sapphire
;; URL: https://sapphire-lang.org

;;; Commentary:
;; Provides syntax highlighting, indentation, and basic navigation
;; for Sapphire (.spp) source files.
;;
;; Installation:
;;   (add-to-list 'load-path "/path/to/editors/emacs")
;;   (require 'sapphire-mode)

;;; Code:

(defgroup sapphire nil
  "Major mode for editing Sapphire source code."
  :group 'languages)

;; ── Keyword lists ──────────────────────────────────────────────────────────

(defconst sapphire-keywords-control
  '("if" "else" "elif" "for" "while" "break" "continue" "return"
    "match" "case" "default" "in" "pass" "del"))

(defconst sapphire-keywords-declaration
  '("fn" "class" "trait" "impl" "macro" "enum"))

(defconst sapphire-keywords-other
  '("import" "from" "as" "let" "var" "const"
    "pub" "priv" "static" "mut" "owned" "where" "dyn" "Self"))

(defconst sapphire-keywords-async
  '("async" "await" "chan" "go" "select"))

(defconst sapphire-keywords-exception
  '("try" "catch" "finally" "throw" "raise"))

(defconst sapphire-types
  '("int" "float" "double" "str" "bool" "void" "any"
    "list" "dict" "tuple" "set" "bytes" "char"))

(defconst sapphire-constants
  '("true" "false" "none" "None" "Some" "Ok" "Err"))

;; ── Font-lock rules ────────────────────────────────────────────────────────

(defconst sapphire-font-lock-keywords
  `(
    ;; Doc comments
    ("///.*$"                                    . font-lock-doc-face)
    ("#!.*$"                                     . font-lock-doc-face)

    ;; Regular comments
    ("#[^!\n][^\n]*$"                            . font-lock-comment-face)
    ("#$"                                        . font-lock-comment-face)

    ;; Decorators
    ("@[a-zA-Z_][a-zA-Z0-9_]*"                  . font-lock-preprocessor-face)

    ;; F-strings (mark the prefix)
    ("\\bf\\(\"[^\"]*\"\\|'[^']*'\\)"           . font-lock-string-face)

    ;; Strings
    ("\"[^\"]*\""                                . font-lock-string-face)
    ("'[^']*'"                                   . font-lock-string-face)

    ;; Numbers — hex, binary, octal, float, int
    ("\\b0[xX][0-9a-fA-F_]+"                    . font-lock-constant-face)
    ("\\b0[bB][01_]+"                            . font-lock-constant-face)
    ("\\b0[oO][0-7_]+"                           . font-lock-constant-face)
    ("\\b[0-9][0-9_]*\\.[0-9][0-9_]*\\b"        . font-lock-constant-face)
    ("\\b[0-9][0-9_]*\\b"                        . font-lock-constant-face)

    ;; Constants
    (,(regexp-opt sapphire-constants 'words)     . font-lock-constant-face)

    ;; Types
    (,(regexp-opt sapphire-types 'words)         . font-lock-type-face)

    ;; PascalCase type names
    ("\\b[A-Z][a-zA-Z0-9_]*\\b"                 . font-lock-type-face)

    ;; Declaration keywords + name
    ("\\b\\(fn\\)\\s-+\\([a-zA-Z_][a-zA-Z0-9_]*\\)"
     (1 font-lock-keyword-face)
     (2 font-lock-function-name-face))
    ("\\b\\(class\\|trait\\|impl\\|enum\\)\\s-+\\([A-Za-z_][A-Za-z0-9_]*\\)"
     (1 font-lock-keyword-face)
     (2 font-lock-type-face))

    ;; Function calls
    ("\\b\\([a-zA-Z_][a-zA-Z0-9_]*\\)\\s-*("   1 font-lock-function-name-face)

    ;; Control keywords
    (,(regexp-opt sapphire-keywords-control 'words)     . font-lock-keyword-face)
    (,(regexp-opt sapphire-keywords-declaration 'words) . font-lock-keyword-face)
    (,(regexp-opt sapphire-keywords-other 'words)       . font-lock-keyword-face)
    (,(regexp-opt sapphire-keywords-async 'words)       . font-lock-keyword-face)
    (,(regexp-opt sapphire-keywords-exception 'words)   . font-lock-keyword-face)

    ;; Operators
    ("=>"                                        . font-lock-builtin-face)
    ("->"                                        . font-lock-builtin-face)
    ("<-"                                        . font-lock-builtin-face)
    ))

;; ── Syntax table ──────────────────────────────────────────────────────────

(defvar sapphire-mode-syntax-table
  (let ((st (make-syntax-table)))
    ;; # starts a comment
    (modify-syntax-entry ?# "<" st)
    (modify-syntax-entry ?\n ">" st)
    ;; Strings
    (modify-syntax-entry ?\" "\"" st)
    (modify-syntax-entry ?\' "\"" st)
    ;; Underscore is part of words
    (modify-syntax-entry ?_ "w" st)
    st)
  "Syntax table for `sapphire-mode'.")

;; ── Indentation ───────────────────────────────────────────────────────────

(defcustom sapphire-indent-offset 4
  "Number of spaces per indentation level in Sapphire."
  :type 'integer
  :group 'sapphire)

(defun sapphire-indent-line ()
  "Indent current line as Sapphire code."
  (interactive)
  (let ((indent (sapphire-calculate-indent)))
    (when indent
      (save-excursion
        (beginning-of-line)
        (delete-horizontal-space)
        (indent-to indent)))))

(defun sapphire-calculate-indent ()
  "Calculate the indentation for the current line."
  (save-excursion
    (beginning-of-line)
    (if (bobp)
        0
      (let ((prev-indent 0))
        (forward-line -1)
        (setq prev-indent (current-indentation))
        ;; Increase indent after lines ending with ':'
        (end-of-line)
        (if (looking-back ":\\s-*" (line-beginning-position))
            (+ prev-indent sapphire-indent-offset)
          prev-indent)))))

;; ── Mode definition ───────────────────────────────────────────────────────

;;;###autoload
(define-derived-mode sapphire-mode prog-mode "Sapphire"
  "Major mode for editing Sapphire source files."
  :syntax-table sapphire-mode-syntax-table
  (setq-local comment-start "# ")
  (setq-local comment-end "")
  (setq-local indent-line-function #'sapphire-indent-line)
  (setq-local font-lock-defaults '(sapphire-font-lock-keywords nil nil nil nil)))

;;;###autoload
(add-to-list 'auto-mode-alist '("\\.spp\\'" . sapphire-mode))

(provide 'sapphire-mode)
;;; sapphire-mode.el ends here
