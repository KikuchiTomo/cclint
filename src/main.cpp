#include "cli/argument_parser.hpp"
#include "cli/help_formatter.hpp"
#include "compiler/detector.hpp"
#include "compiler/wrapper.hpp"
#include "config/config_loader.hpp"
#include "diagnostic/diagnostic.hpp"
#include "output/formatter_factory.hpp"
#include "utils/logger.hpp"
#include "engine/analysis_engine.hpp"

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
        if (args.max_errors > 0) {
            config.max_errors = args.max_errors;
        }
        if (args.num_threads > 0) {
            config.num_threads = args.num_threads;
        }
        config.enable_cache = args.enable_cache;

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

        // 解析エンジンの初期化
        logger.info("Initializing analysis engine...");
        cclint::engine::AnalysisEngine analysis_engine(config);

        // ソースファイルの解析
        if (!result.source_files.empty()) {
            logger.info("Analyzing " +
                        std::to_string(result.source_files.size()) +
                        " source file(s)...");

            auto analysis_results =
                analysis_engine.analyze_files(result.source_files);

            // 解析結果のサマリーをログ出力
            size_t success_count = 0;
            size_t failed_count = 0;
            for (const auto& res : analysis_results) {
                if (res.success) {
                    success_count++;
                } else {
                    failed_count++;
                    logger.error("Failed to analyze: " + res.file_path +
                                 " - " + res.error_message);
                }
            }

            logger.info("Successfully analyzed " +
                        std::to_string(success_count) + " file(s)");
            if (failed_count > 0) {
                logger.warning("Failed to analyze " +
                               std::to_string(failed_count) + " file(s)");
            }

        } else {
            logger.warning("No source files found to analyze");
        }

        // すべての診断を取得
        auto all_diagnostics = analysis_engine.get_all_diagnostics();

        // 出力フォーマッタの作成
        auto formatter =
            output::FormatterFactory::create(config.output_format);

        // 診断結果の出力
        formatter->format(all_diagnostics, std::cout);

        // 統計情報の表示
        if (args.verbosity > 0) {
            logger.info("Analysis complete");

            auto stats = analysis_engine.get_stats();
            logger.info("Files analyzed: " +
                        std::to_string(stats.analyzed_files) + "/" +
                        std::to_string(stats.total_files));

            if (stats.skipped_files > 0) {
                logger.info("Files skipped: " +
                            std::to_string(stats.skipped_files));
            }

            if (stats.failed_files > 0) {
                logger.warning("Files failed: " +
                               std::to_string(stats.failed_files));
            }

            if (stats.stopped_early) {
                logger.warning("Analysis stopped early (max_errors reached)");
            }

            logger.info(
                "Errors: " + std::to_string(analysis_engine.get_error_count()));
            logger.info("Warnings: " +
                        std::to_string(analysis_engine.get_warning_count()));

            if (args.verbosity > 1) {
                logger.info("Total time: " +
                            std::to_string(stats.total_time.count()) + "ms");
            }
        }

        // 終了コードの決定
        if (analysis_engine.get_error_count() > 0) {
            return 1;  // エラーがある場合
        }

        logger.info("cclint finished successfully");
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 2;
    }
}
