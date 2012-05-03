module JSOS
  class BinaryCompiler
    attr_accessor :bytecode, :ast
  
    def initialize(ast, filename = "")
      @ast = ast
      @sections = [[]]
      @section_stack = [0]
      @scope_stack = []
      @interned_strings = {}
      @continue_stack = []
      @break_stack = []
      @label_ai = 0
      @filename = filename
    end
  
    def compile
      ast.each &method(:hoist)
      ast.each &method(:compile_node)
      output :undefined
      output :ret
      generate_bytecode
    end
  
    OPCODES = {
      undefined:  0,
      ret:        1,
      pushnum:    2,
      add:        3,
      pushglobal: 4,
      pushstr:    5,
      methcall:   6,
      setvar:     7,
      pushvar:    8,
      true:       9,
      false:      10,
      null:       11,
      jmp:        12,
      jit:        13,
      jif:        14,
      sub:        15,
      mul:        16,
      div:        17,
      setglobal:  18,
      close:      19,
      call:       20,
      setcallee:  21,
      setarg:     22,
      lt:         23,
      lte:        24,
      gt:         25,
      gte:        26,
      pop:        27,
      array:      28,
      newcall:    29,
      throw:      30,
      member:     31,
      dup:        32,
      this:       33,
      setprop:    34,
      tst:        35,
      tld:        36,
      index:      37,
      setindex:   38,
      object:     39,
      typeof:     40,
      seq:        41,
      typeofg:    42,
      sal:        43,
      or:         44,
      xor:        45,
      and:        46,
      slr:        47,
      not:        48,
      bitnot:     49,
      line:       50,
      debugger:   51,
      instanceof: 52,
      negate:     53,
      try:        54,
      poptry:     55,
      catch:      56,
      catchg:     57,
      popcatch:   58,
      finally:    59,
      popfinally: 60,
      closenamed: 61,
    }

  private

    def generate_bytecode
      @bytecode = "JSX\0"
      bytecode << [intern_string(@filename)].pack("L<")
      # how many sections exist as LE uint32_t:
      bytecode << [@sections.size].pack("L<")
      @sections.map(&method(:generate_bytecode_for_section)).each do |sect|
        bytecode << [sect.size].pack("L<") << sect
      end
      bytecode << [@interned_strings.count].pack("L<")
      @interned_strings.each do |str,idx|
        bytecode << [str.length].pack("L<")
        # this hack here is so that UTF-8 doesn't break stuff like "\xEF\xBE\xAD\xDE":
        bytecode << str.each_char.map(&:ord).map(&:chr).join << "\0"
      end
    end
  
    def generate_bytecode_for_section(section)
      acc = 0
      label_refs = {}
      section.each do |sect|
        if sect.is_a? Array
          fixup, arg = sect
          case fixup
          when :label;  label_refs[arg] = acc / 4
          when :ref;    acc += 4
          end
        else
          acc += sect.bytes.count
        end
      end
      section.reject { |a,b| a == :label }.map { |x| x.is_a?(Array) ? [label_refs[x[1]]].pack("L<") : x }.join
    end

    def current_section
      @sections[@section_stack.last]
    end

    def push_section(section = nil)
      unless section
        @section_stack << @sections.size
        @sections << []
      else
        @section_stack << section
      end  
      @section_stack.last
    end
  
    def pop_section
      @section_stack.pop
    end
  
    def push_scope
      @scope_stack << { }
    end
  
    def pop_scope
      @scope_stack.pop
    end
  
    def create_local_var(var)
      if @scope_stack.any?
        @scope_stack.last[var] ||= @scope_stack.last.count
      end
    end
  
    def lookup_var(var)
      @scope_stack.reverse_each.each_with_index do |scope, index|
        return [scope[var], index] if scope[var]
      end
      nil
    end

    def compile_node(node)
      if @current_line != node.line
        @current_line = node.line
        output :line, @current_line
      end
      if respond_to? type(node), true
        send type(node), node
      else
        raise "unimplemented node type #{type(node)} - line #{node.line}"
      end
    end
  
    def intern_string(str)
      @interned_strings[str] ||= @interned_strings.count
    end
  
    def output(*ops)
      ops.each do |op|
        case op
        when Symbol
          raise "unknown op #{op.inspect}" unless OPCODES[op]
          current_section << [OPCODES[op]].pack("L<")
        when Float
          current_section << [op].pack("E")
        when Fixnum
          current_section << [op].pack("L<")
        when String
          current_section << [intern_string(op)].pack("L<")
        when Array
          current_section << op # this is a fixup to be resolved later
        else
          raise "bad op type #{op.class.name}"
        end
      end
    end
  
    def type(node)
      node.class.name.split("::").last.intern
    end
  
    def hoist(node)
      node.walk do |node|
        if node.is_a? Twostroke::AST::Declaration
          create_local_var node.name
        elsif node.is_a? Twostroke::AST::Function
          if node.name
            create_local_var node.name
            # because javascript is odd, entire function bodies need to be hoisted, not just their declarations
            Function(node, true)
          end
          false
        else
          true
        end
      end
    end
  
    def uniqid
      @label_ai += 1
    end
  
    # ast node compilers
  
    { Addition: :add, Subtraction: :sub, Multiplication: :mul, Division: :div,
      Equality: :eq, StrictEquality: :seq, LessThan: :lt, GreaterThan: :gt,
      LessThanEqual: :lte, GreaterThanEqual: :gte, BitwiseAnd: :and,
      BitwiseOr: :or, BitwiseXor: :xor, In: :in, RightArithmeticShift: :sar,
      LeftShift: :sal, RightLogicalShift: :slr, InstanceOf: :instanceof,
      Modulus: :mod
    }.each do |method,op|
      define_method method do |node|
        if node.assign_result_left
          if type(node.left) == :Variable || type(node.left) == :Declaration
            compile_node node.left
            compile_node node.right
            output op
            idx, sc = lookup_var node.left.name
            if idx
              output :setvar, idx, sc
            else
              output :setglobal, node.left.name
            end
          elsif type(node.left) == :MemberAccess
            compile_node node.left.object
            output :dup
            output :member, node.left.member
            compile_node node.right
            output op
            output :setprop, node.left.member          
          elsif type(node.left) == :Index
            compile_node node.left.object
            compile_node node.left.index
            output :dupn, 2
            output :index
            compile_node node.right
            output op
            output :setindex
          else
            error! "Bad lval in combined operation/assignment"
          end
        else
          compile_node node.left
          compile_node node.right
          output op
        end
      end
      private method
    end
    
    def BinaryNot(node)
      compile_node node.value
      output :bitnot
    end
    
    def Negation(node)
      compile_node node.value
      output :negate
    end
    
    def Debugger(node)
      output :debugger
    end
    
    def StrictInequality(node)
      StrictEquality(node)
      output :not
    end
  
    def post_mutate(left)
      if type(left) == :Variable || type(left) == :Declaration
        if var = lookup_var(left.name)
          output :pushvar, *var
        else
          output :pushglobal, left.name
        end
        output :dup
        yield
        if var
          output :setvar, *var
        else
          output :setglobal, left.name
        end
        output :pop
      elsif type(left) == :MemberAccess
        compile_node left.object
        output :dup
        output :member, left.member
        output :dup
        output :tst
        yield
        output :setprop, left.member
        output :pop
        output :tld
      elsif type(left) == :Index  
        compile_node left.object
        compile_node left.index
        output :dupn, 2
        output :index
        output :dup
        output :tst
        yield
        output :setindex
        output :pop
        output :tld
      else
        error! "Bad lval in post-mutation"
      end
    end
  
    def PostIncrement(node)
      post_mutate node.value do
        output :pushnum, -1.0
        output :sub
      end
    end
  
    def PostDecrement(node)
      post_mutate node.value do
        output :pushnum, 1.0
        output :sub
      end
    end
    
    def pre_mutate(left)
      if type(left) == :Variable || type(left) == :Declaration
        if var = lookup_var(left.name)
          output :pushvar, *var
        else
          output :pushglobal, left.name
        end
        yield
        if var
          output :setvar, *var
        else
          output :setglobal, left.name
        end
      elsif type(left) == :MemberAccess
        compile_node left.object
        output :dup
        output :member, left.member
        yield
        output :setprop, left.member
      elsif type(left) == :Index
        compile_node left.object
        compile_node left.index
        output :dup, 2
        output :index
        yield
        output :setindex
      else
        error! "Bad lval in post-mutation"
      end
    end

    def PreIncrement(node)
      pre_mutate node.value do
        output :pushnum, -1.0
        output :sub
      end
    end

    def PreDecrement(node)
      pre_mutate node.value do
        output :pushnum, 1.0
        output :sub
      end
    end
  
    def Function(node, in_hoist_stage = false)
      fnid = node.fnid
    
      if !node.name or in_hoist_stage
        if fnid
          push_section(fnid)
        else
          node.fnid = fnid = push_section
        end
        push_scope
        output :setcallee, create_local_var(node.name) if node.name
        node.arguments.each_with_index do |arg, idx|
          output :setarg, create_local_var(arg), idx
        end
        output :line, node.line
        node.statements.each { |s| hoist s }
        node.statements.each { |s| compile_node s }
        pop_scope
        output :undefined
        output :ret
        pop_section
        if node.name
          output :closenamed, fnid, node.name
        else
          output :close, fnid
        end
        if node.name && !node.as_expression
          if idx = create_local_var(node.name)
            output :setvar, create_local_var(node.name), 0 
          else
            output :setglobal, node.name
          end
        end
      else
        if node.name
          output :closenamed, fnid, node.name
        else
          output :close, fnid
        end
      end
    end
  
    def MultiExpression(node)
      compile_node node.left
      output :pop unless node.left.is_a? Twostroke::AST::Declaration
      compile_node node.right
    end
  
    def Variable(node)
      idx, sc = lookup_var node.name
      if idx
        output :pushvar, idx, sc
      else
        output :pushglobal, node.name
      end
    end
  
    def Number(node)
      output :pushnum, node.number.to_f
    end
  
    def ObjectLiteral(node)
      node.items.each do |k,v|
        output :pushstr, k.val.to_s
        compile_node v
      end
      output :object, node.items.count
    end
  
    def Array(node)
      node.items.each do |item|
        compile_node item
      end
      output :array, node.items.count
    end
  
    def String(node)
      output :pushstr, node.string
    end
  
    def MemberAccess(node)
      compile_node node.object
      output :member, node.member
    end
  
    def Index(node)
      compile_node node.object
      compile_node node.index
      output :index
    end
  
    def New(node)
      compile_node node.callee
      node.arguments.each { |n| compile_node n }
      output :newcall, node.arguments.size
    end
  
    def Call(node)
      if type(node.callee) == :MemberAccess
        compile_node node.callee.object
        output :pushstr, node.callee.member.to_s
        node.arguments.each { |n| compile_node n }
        output :methcall, node.arguments.size
      elsif type(node.callee) == :Index
        compile_node node.callee.object
        compile_node node.callee.index
        node.arguments.each { |n| compile_node n }
        output :methcall, node.arguments.size
      else
        compile_node node.callee
        node.arguments.each { |n| compile_node n }
        output :call, node.arguments.size
      end
    end
  
    def TypeOf(node)
      if node.value.is_a? Twostroke::AST::Variable and not lookup_var node.value.name
        output :typeofg, node.value.name
      else
        compile_node node.value
        output :typeof
      end
    end
  
    def Declaration(node)
      # no-op
    end
  
    def Assignment(node)
      if type(node.left) == :Variable || type(node.left) == :Declaration
        compile_node node.right
        idx, sc = lookup_var node.left.name
        if idx
          output :setvar, idx, sc
        else
          output :setglobal, node.left.name
        end
      elsif type(node.left) == :MemberAccess
        compile_node node.left.object
        compile_node node.right
        output :setprop, node.left.member
      elsif type(node.left) == :Index
        compile_node node.left.object
        compile_node node.left.index
        compile_node node.right
        output :setindex
      else  
        error! "Bad lval in assignment"
      end
    end

    def Throw(node)
      compile_node node.expression
      output :throw
    end
  
    def Return(node)
      if node.expression
        compile_node node.expression
      else
        output :undefined
      end
      output :ret
    end
  
    def If(node)
      compile_node node.condition
      else_label = uniqid
      output :jif, [:ref, else_label]
      compile_node node.then
      if node.else
        end_label = uniqid
        output :jmp, [:ref, end_label]
        output [:label, else_label]
        compile_node node.else
        output [:label, end_label]
      else
        output [:label, else_label]
      end
    end
    
    def Ternary(node)
      compile_node node.condition
      else_label = uniqid
      end_label = uniqid
      output :jif, [:ref, else_label]
      compile_node node.if_true
      output :jmp, [:ref, end_label]
      output [:label, else_label]
      compile_node node.if_false
      output [:label, end_label]
    end
    
    def While(node, continue_label = nil)
      start_label = uniqid
      end_label = uniqid
      @continue_stack.push start_label
      @break_stack.push end_label
      output [:label, start_label]
      output [:label, continue_label] if continue_label
      compile_node node.condition
      output :jif, [:ref, end_label]
      compile_node node.body if node.body
      output :jmp, [:ref, start_label]
      output [:label, end_label]
      @continue_stack.pop
      @break_stack.pop
    end
    
    def DoWhile(node, continue_label = nil)
      start_label = uniqid
      end_label = uniqid
      @continue_stack.push start_label
      @break_stack.push end_label
      output [:label, start_label]
      output [:label, continue_label] if continue_label
      compile_node node.condition
      output :jif, [:ref, end_label]
      compile_node node.body if node.body
      output :jmp, [:ref, start_label]
      output [:label, end_label]
      @continue_stack.pop
      @break_stack.pop
    end
  
    def ForLoop(node, continue_label = nil)
      compile_node node.initializer if node.initializer
      start_label = uniqid
      next_label = uniqid
      end_label = uniqid
      output [:label, start_label]
      if node.condition
        compile_node node.condition
        output :jif, [:ref, end_label]
      end
      @continue_stack.push next_label
      @break_stack.push end_label
      compile_node node.body if node.body
      output [:label, next_label]
      compile_node node.increment if node.increment
      output :jmp, [:ref, start_label]
      output [:label, end_label]
      @continue_stack.pop
      @break_stack.pop
    end
    
    def Try(node)
      catch_label = uniqid
      finally_label = uniqid
      output :try, [:ref, catch_label], [:ref, finally_label]
      node.try_statements.each { |n| compile_node n }
      output :poptry

      output [:label, catch_label]
      if node.catch_statements
        if @scope_stack.any?
          output :catch, create_local_var(node.catch_variable)
        else
          output :catchg, node.catch_variable
        end
        node.catch_statements.each { |n| compile_node n }
        output :popcatch
      end

      output [:label, finally_label]
      output :finally
      node.finally_statements.each { |n| compile_node n } if node.finally_statements
      output :popfinally
    end
    
    def Break(node)
      if node.label
        error! "Undefined label '#{node.label}'" unless @labels[node.label]
        output :jmp, [:ref, @labels[node.label][:break]]
      else
        error! "Break not allowed outside of loop" unless @break_stack.any?
        output :jmp, [:ref, @break_stack.last]
      end
    end

    def Continue(node)
      if node.label
        error! "Undefined label '#{node.label}'" unless @labels[node.label]
        output :jmp, [:ref, @labels[node.label][:continue]]
      else
        error! "Continue not allowed outside of loop" unless @continue_stack.any?
        output :jmp, [:ref, @continue_stack.last]
      end
    end
  
    def Or(node)
      compile_node node.left
      output :dup
      end_label = uniqid
      output :jit, [:ref, end_label]
      output :pop
      compile_node node.right
      output [:label, end_label]
    end
  
    def And(node)
      compile_node node.left
      output :dup
      end_label = uniqid
      output :jif, [:ref, end_label]
      output :pop
      compile_node node.right
      output [:label, end_label]
    end
  
    def Body(node)
      node.statements.each do |n|
        compile_node n
      end
    end
  
    def True(node)
      output :true
    end
  
    def False(node)
      output :false
    end
  
    def Null(node)
      output :null
    end
  
    def This(node)
      output :this
    end
    
    def Void(node)
      compile_node node.value
      output :pop
      output :undefined
    end
    
    def Not(node)
      compile_node node.value
      output :not
    end
  end
end