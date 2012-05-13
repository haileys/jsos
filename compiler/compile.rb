$LOAD_PATH << File.expand_path("../../twostroke/lib", __FILE__)
require "twostroke"
require File.expand_path("../jsx_compiler", __FILE__)

file = File.open ARGV.first, "r"
parser = Twostroke::Parser.new Twostroke::Lexer.new file.read
parser.parse

compiler = JSOS::JSXCompiler.new parser.statements, ARGV.last
compiler.compile

print compiler.bytecode