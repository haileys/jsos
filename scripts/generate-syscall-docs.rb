# coding: utf-8

puts "# JSOS System Calls"

lines = File.readlines "kernel/js/kernel/process.js"

SYSCALL_RE      = %r{g\.OS\.(?<name>[a-z0-9_]+) =}i
ARGS_RE         = %r{exposeFunction\(function\((?<args>[a-z0-9_,\s]*)\)}
DOC_COMMENT_RE  = %r{^\s+(//(?<comment>.*))?$}
DOC_FIELD_RE    = %r{^\s*(?<field_name>[a-z0-9_]+):}

lines.each_with_index.select { |line, idx| line =~ SYSCALL_RE }.each do |defn, line|
  syscall = defn.match(SYSCALL_RE)[:name]
  next unless defn.include? "exposeFunction"
  
  # extract arguments out of function
  args = defn.match(ARGS_RE)[:args].split(",").map(&:strip)
  
  # find doc comments
  comment = lines[0...line].reverse_each.take_while { |s| s =~ DOC_COMMENT_RE }.reverse.map { |s| s.match(DOC_COMMENT_RE)[:comment] || "" }
  
  description, *fields_raw = comment.slice_before { |l| l =~ DOC_FIELD_RE }.to_a
  description ||= ["undocumented"]
  fields = Hash[fields_raw.map { |x| x.join(" ").match(DOC_FIELD_RE) }.map { |md| [md[:field_name].intern, md.post_match.strip] }]
  
  str = "### #{syscall}(#{args.map { |a| "`#{a}`" }.join ", "})\n\n"
  description.each do |l|
    str << "> #{l.strip}\n"
  end
  str << "\n"
  if fields.any?
    fields.each do |k, v|
      str << "> **#{k}:** #{v}\n\n"
    end
  end
  str << "[Source](../kernel/js/kernel/process.js#L#{line + 1})\n\n"
  
  puts str
end