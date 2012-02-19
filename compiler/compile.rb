$LOAD_PATH << File.expand_path("../../twostroke/lib", __FILE__)
require "twostroke"
require File.expand_path("../binary_compiler", __FILE__)

file = File.open ARGV.first, "r:utf-8"
parser = Twostroke::Parser.new Twostroke::Lexer.new file.read
parser.parse

compiler = JSOS::BinaryCompiler.new parser.statements
compiler.compile

print compiler.bytecode