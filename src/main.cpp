#include "cli/argument_parser.hpp"
#include "cli/help_formatter.hpp"
#include "compiler/detector.hpp"
#include "compiler/wrapper.hpp"
#include "config/config_loader.hpp"
#include "diagnostic/diagnostic.hpp"
#include "output/formatter_factory.hpp"
#include "utils/logger.hpp"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    try {
        // コマンドライン引数解析
        cli::ArgumentParser parser;
        auto args = parser.parse(argc, argv);

        // --help
        if (args.show_help || (argc == 1)) {
            std::cout << cli::HelpFormatter::format_help();
            return 0;
        }

        // --version
        if (args.show_version) {
            std::cout << cli::HelpFormatter::format_version();
            return 0;
        }

        // ロガーの初期化
        utils::Logger& logger = utils::Logger::instance();
        if (args.verbosity == 0) {
            logger.set_level(utils::LogLevel::Error);
        } else if (args.verbosity == 1) {
            logger.set_level(utils::LogLevel::Info);
        } else {
            logger.set_level(utils::LogLevel::Debug);
        }

        logger.info("cclint starting...");

        // 設定ファイルの読み込み
        config::ConfigLoader config_loader;
        config::Config config;

        if (args.config_file) {
            logger.info("Loading config from: " + *args.config_file);
            config = config_loader.load_from_file(*args.config_file);
        } else {
            logger.info("Using default config or searching for config file");
            config = config_loader.load(std::nullopt, ".");
            auto loaded_path = config_loader.get_loaded_config_path();
            if (loaded_path) {
                logger.info("Loaded config from: " + *loaded_path);
            } else {
                logger.info("Using default configuration");
            }
        }

        // コマンドラインオプションで設定を上書き
        if (!args.output_format.empty()) {
            config.output_format = args.output_format;
        }

        // コンパイラコマンドが指定されていなければエラー
        if (args.compiler_command.empty()) {
            std::cerr << "Error: No compiler command specified\n";
            std::cerr << "Usage: cclint [OPTIONS] <compiler-command>\n";
            std::cerr << "Try 'cclint --help' for more information.\n";
            return 1;
        }

        // コンパイラの検出
        compiler::CompilerDetector detector;
        auto compiler_info = detector.detect(args.compiler_command);
        if (compiler_info.type != compiler::CompilerType::Unknown) {
            logger.info("Detected compiler: " + compiler_info.name + " version " +
                        compiler_info.version);
        } else {
            logger.warning("Could not detect compiler type");
        }

        // コンパイラコマンドの実行
        logger.info("Executing compiler command...");
        compiler::CompilerWrapper wrapper(args.compiler_command);
        auto result = wrapper.execute();

        logger.info("Compiler exit code: " + std::to_string(result.exit_code));

        // ソースファイルの表示
        if (!result.source_files.empty()) {
            logger.info("Source files found:");
            for (const auto& file : result.source_files) {
                logger.info("  - " + file);
            }
        }

        // コンパイラフラグの表示
        if (!result.compiler_flags.empty() && args.verbosity > 1) {
            logger.debug("Compiler flags:");
            for (const auto& flag : result.compiler_flags) {
                logger.debug("  - " + flag);
            }
        }

        // コンパイラ出力の表示（設定による）
        if (config.show_compiler_output) {
            if (!result.stdout_output.empty()) {
                std::cout << result.stdout_output;
            }
            if (!result.stderr_output.empty()) {
                std::cerr << result.stderr_output;
            }
        }

        // 診断エンジンの初期化
        diagnostic::DiagnosticEngine diag_engine;

        // TODO: ここでパーサーとルールエンジンを統合
        // 現在はダミーの診断を生成（開発中であることを示す）
        logger.warning(
            "AST parsing and rule execution not yet implemented");
        logger.info("Required dependencies for full functionality:");
        logger.info("  - LLVM/Clang libtooling (for C++ parsing)");
        logger.info("  - LuaJIT 2.1 (for rule execution)");
        logger.info("  - yaml-cpp (for YAML config parsing)");

        // 出力フォーマッタの作成
        auto formatter =
            output::FormatterFactory::create(config.output_format);

        // 診断結果の出力
        formatter->format(diag_engine.get_diagnostics(), std::cout);

        // 統計情報の表示
        if (args.verbosity > 0) {
            logger.info("Analysis complete");
            logger.info("Errors: " + std::to_string(diag_engine.error_count()));
            logger.info("Warnings: " +
                        std::to_string(diag_engine.warning_count()));
        }

        // 終了コードの決定
        if (diag_engine.error_count() > 0) {
            return 1;  // エラーがある場合
        }

        logger.info("cclint finished successfully");
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 2;
    }
}
