lex = (str) ->
    return ["end_of_input"] if str.length == 0
    
    if str.substr(0, 5) is "d/dx "
        return ["d/dx", null, lex str.substr(5)]
        
    c = str[0]
    
    if c is " " or c is "\n" or c is "\t"
        return lex str.substr(1)
        
    if c >= "1" and c <= "9"
        for i in [1..str.length]
            break unless (str[i] >= "1" and str[i] <= "9") or str[i] is "."
        return ["number", Number(str.substr(0, i)), lex(str.substr(i))]
    
    if (c >= "a" and c <= "z") or (c >= "A" and c <= "Z")
        for i in [1..str.length]
            break unless (str[i] >= "a" and str[i] <= "z") or (str[i] >= "A" and str[i] <= "Z")
        if str[i] is "("
            return ["function", str.substr(0, i + 1), lex(str.substr(i + 1))]
        else
            return ["variable", str.substr(0, i), lex(str.substr(i))]
        
    punct = 
            "+": "plus"
            "-": "minus"
            "*": "times"
            "/": "divide"
            "^": "power"
            "(": "open_paren"
            ")": "close_paren"
            
    if punct[c]
        return [punct[c], null, lex(str.substr(1))]

class Parser
    class @Error extends Error
        constructor: (@message) ->
    
    constructor: (tokens) -> @tokens = [null, null, tokens]
    
    token: -> @tokens[0..1]
    peek_token: -> @tokens[2][0..1]
    next_token: ->
        @tokens = @tokens[2]
        @token()
    
    expect_token: (types...) ->
        [type, val] = @next_token()
        for t in types
            return [type, val] if t == type
        throw new Parser.Error "Expected one of #{types.join(", ")}; got #{type}"
        
    functions: {}
    
    parse: ->
        if @peek_token()[0] is "d/dx"
            expr = @derivative_expression()
        else
            expr = @expression()
        @expect_token "end_of_input"
        expr
    
    derivative_expression: ->
        @expect_token "d/dx"
        new AST.Derivative @expression()
    
    expression: ->
        @additive_expression()
    
    additive_expression: ->
        left = @multiplicative_expression()
        while @peek_token()[0] is "plus" or @peek_token()[0] is "minus"
            [type] = @next_token()
            klass = { plus: AST.Addition, minus: AST.Subtraction }[type]
            left = new klass left, @multiplicative_expression()
        left

    multiplicative_expression: ->
        left = @power_expression()
        while @peek_token()[0] is "times" or @peek_token()[0] is "divide"
            [type] = @next_token()
            klass = { times: AST.Multiplication, divide: AST.Division }[type]
            left = new klass left, @power_expression()
        left
    
    power_expression: ->
        left = @unary_expression()
        if @peek_token()[0] is "power"
            @next_token()
            left = new AST.Power left, @power_expression()
        left
    
    unary_expression: ->
        [type, val] = @expect_token "number", "minus", "open_paren", "function", "variable"
        if type is "number"
            new AST.Number val
        else if type is "minus"
            new AST.Negation @unary_expression()
        else if type is "open_paren"
            expr = @expression()
            @expect_token "close_paren"
            expr
        else if type is "function"
            # TODO
        else if type is "variable"
            if val is "x"
                new AST.X
            else
                throw new Parser.Error "undefined variable #{val}"
            

AST =
    Number: class
        precedence: 0
        constructor: (@number) ->
        derive: ->
            new AST.Number 0
        simplify: -> @
        identical: (other) ->
            other instanceof AST.Number and other.number is @number
        toString: ->
            String @number
            
    X: class
        precedence: 0
        constructor: (@name) ->
        derive: ->
            new AST.Number 1
        simplify: -> @
        identical: (other) ->
            other instanceof AST.X
        toString: -> "x"
        
    Negation: class
        precedence: 1
        constructor: (@expr) ->
        derive: ->
            new AST.Negate @expr.derive()
        simplify: ->
            if @expr instanceof AST.Negation
                @expr.expr.simplify()
            else if @expr instanceof AST.Number
                new AST.Number -@expr.number
            else
                new AST.Negation @expr.simplify()
        identical: (other) ->
            other instanceof AST.Negation and @expr.identical(other.expr)
        toString: ->
            "(#{@expr})"
                
    Addition: class
        precedence: 4
        constructor: (@left, @right) ->
        derive: ->
            new AST.Addition @left.derive(), @right.derive()
        simplify: ->
            left = @left.simplify()
            right = @right.simplify()
            if left.identical right
                new AST.Multiplication(new AST.Number(2), left).simplify()
            else if left instanceof AST.Number and left.number is 0
                right
            else if right instanceof AST.Number and right.number is 0
                left
            else if left instanceof AST.Number and right instanceof AST.Number
                new AST.Number left.number + right.number
            else
                new AST.Addition left, right
        identical: (other) ->
            other instanceof AST.Addition and @left.identical(other.left) and @right.identical(other.right)
        toString: ->
            "(#{@left}) + (#{@right})"
                
    Subtraction: class
        precedence: 4
        constructor: (@left, @right) ->
        derive: ->
            new AST.Subtraction @left.derive(), @right.derive()
        simplify: ->
            left = @left.simplify()
            right = @right.simplify()
            if left instanceof AST.Number and left.number is 0
                new AST.Negation(right).simplify()
            else if right instanceof AST.Number and right.number is 0
                left
            else if left instanceof AST.Number and right instanceof AST.Number
                new AST.Number left.number - right.number
            else
                new AST.Addition left, right
        identical: (other) ->
            other instanceof AST.Subtraction and @left.identical(other.left) and @right.identical(other.right)
        toString: ->
            "(#{@left}) - (#{@right})"
        
    Multiplication: class
        precedence: 3
        constructor: (@left, @right) ->
        derive: ->
            new AST.Addition new AST.Multiplication(@left.derive(), @right), new AST.Multiplication(@left, @right.derive())
        simplify: ->
            left = @left.simplify()
            right = @right.simplify()
            if right instanceof AST.Number
                [left, right] = [right, left]
            
            if left instanceof AST.Power and left.right instanceof AST.Number
                if left.left.identical right
                    return new AST.Power(right, new AST.Number left.right.number + 1).simplify()
                if right instanceof AST.Power and right.right instanceof AST.Number
                    if right.left.identical left.left
                        return new AST.Power(left.left, new AST.Number right.right.number + left.right.number).simplify()
            
            if right instanceof AST.Power and right.right instanceof AST.Number
                if right.left.identical left
                    return new AST.Power(left, new AST.Number right.right.number + 1).simplify()
            
            if left.identical right
                new AST.Power(left, new AST.Number 2).simplify()
            else if left instanceof AST.Number and left.number is 0
                left
            else if right instanceof AST.Number and right.number is 0
                right
            else if left instanceof AST.Number and left.number is 1
                right
            else if right instanceof AST.Number and right.number is 1
                left
            else if left instanceof AST.Number and right instanceof AST.Number
                new AST.Number left.number * right.number
            else if left instanceof AST.Negation and right instanceof AST.Negation
                new AST.Multiplication(left.expr, right.expr).simplify()
            else if left instanceof AST.Negation
                new AST.Negation(new AST.Multiplication left.expr, right).simplify()
            else if right instanceof AST.Negation
                new AST.Negation(new AST.Multiplication left, right.expr).simplify()
            else if left instanceof AST.Multiplication
                new AST.Multiplication(left.left, new AST.Multiplication(left.right, right)).simplify()
            else if left instanceof AST.Division and right instanceof AST.Division
                new AST.Division(new AST.Multiplication(left.left, right.left), new AST.Multiplication(left.right, right.right)).simplify()
            else if left instanceof AST.Division
                new AST.Division(new AST.Multiplication(left.left, right), left.right).simplify()
            else if right instanceof AST.Division
                new AST.Division(new AST.Multiplication(left, right.left), right.right).simplify()
            else if left instanceof AST.Number and right instanceof AST.Multiplication and right.left instanceof AST.Number
                new AST.Multiplication(new AST.Number(left.number * right.left.number), right.right).simplify()
            else
                new AST.Multiplication left, right
        identical: (other) ->
            other instanceof AST.Multiplication and @left.identical(other.left) and @right.identical(other.right)
        toString: ->
            "(#{@left}) * (#{@right})"
            
    Division: class
        precedence: 3
        constructor: (@left, @right) ->
        derive: ->
            new AST.Division new AST.Subtraction(new AST.Multiplication(@left.derive(), @right), new AST.Multiplication(@left, @right.derive())), new AST.Power(@right, new AST.Number(2))
        simplify: ->
            left = @left.simplify()
            right = @right.simplify()
            
            if left instanceof AST.Number and right instanceof AST.Number
                gcd = (a, b) ->
                    if b is 0
                        a
                    else
                        gcd b, a % b
                factor = gcd left.number, right.number
                return new AST.Division(new AST.Number(left.number / factor), new AST.Number(right.number / factor)).simplify()
            
            if left instanceof AST.Number and left.number is 0
                new AST.Number 0
            else if right instanceof AST.Number and right.number is 1
                left
            else if right instanceof AST.Division
                new AST.Multiplication(left, new AST.Division(right.right, right.left)).simplify()
            else if left instanceof AST.Division
                new AST.Division(left.left, new AST.Multiplication(left.right, right)).simplify()
            else if left instanceof AST.Negation and right instanceof AST.Negation
                new AST.Division(left.expr, right.expr).simplify()
            else if left instanceof AST.Negation
                new AST.Negation(new AST.Division(left.expr, right)).simplify()
            else if right instanceof AST.Negation
                new AST.Negation(new AST.Division(left, right.expr)).simplify()
            else
                @removeCommonFactors()
        removeCommonFactors: ->
            left = @left.simplify()
            right = @right.simplify()
            top_muls = []
            bottom_muls = []
            top = left
            while top instanceof AST.Multiplication
                top_muls.push top.left
                top = top.right
            top_muls.push top
            bottom = right
            while bottom instanceof AST.Multiplication
                bottom_muls.push bottom.left
                bottom = bottom.right
            bottom_muls.push bottom
            
            for i of top_muls
                for j of bottom_muls
                    if top_muls[i] instanceof AST.Power and top_muls[i].right instanceof AST.Number and top_muls[i].left.identical bottom_muls[j]
                        top_muls[i] = new AST.Power(top_muls[i].left, new AST.Number(top_muls[i].right.number - 1)).simplify()
                        bottom_muls[j] = new AST.Number 1
                    if top_muls[i].identical bottom_muls[j]
                        bottom_muls[j] = top_muls[i] = new AST.Number 1
            
            top = top_muls.reduceRight((b, a) -> new AST.Multiplication(b, a)).simplify()
            bottom = bottom_muls.reduceRight((b, a) -> new AST.Multiplication(b, a)).simplify()
            
            if bottom instanceof AST.Number and bottom.number is 1
                top
            else
                new AST.Division top, bottom
            
        identical: (other) ->
            other instanceof AST.Division and @left.identical(other.left) and @right.identical(other.right)
        toString: ->
            "(#{@left}) / (#{@right})"
            
    Power: class
        precedence: 2
        constructor: (@left, @right) ->
        derive: ->
            pow = (a,b) -> new AST.Power a, b
            mul = (a,b) -> new AST.Multiplication a, b
            div = (a,b) -> new AST.Division a, b
            add = (a,b) -> new AST.Addition a, b
            ln  = (x)   -> new AST.LogNatural x
            f = @left
            g = @right
            fdash = f.derive()
            gdash = g.derive()
            mul(pow(f, g), add(mul(gdash, ln(f)), mul(g, div(fdash, f))))
        simplify: ->
            left = @left.simplify()
            right = @right.simplify()
            if left instanceof AST.Number and right instanceof AST.Number
                new AST.Number Math.pow left.number, right.number
            else if right instanceof AST.Number and right.number is 0
                new AST.Number 1
            else if right instanceof AST.Number and right.number is 1
                left
            else
                new AST.Power left, right
        identical: (other) ->
            other instanceof AST.Power and @left.identical(other.left) and @right.identical(other.right)
        toString: ->
            "(#{@left}) ^ (#{@right})"
        
    LogNatural: class
        precedence: 0
        constructor: (@expr) ->
        derive: ->
            new AST.Division @expr.derive(), @expr
        simplify: ->
            expr = @expr.simplify()
            if expr instanceof AST.Number and node.number is 1
                new AST.Number 0
            else
                new AST.LogNatural expr
        identical: (other) ->
            other instanceof AST.LogNatural and @expr.identical(other.expr)
        toString: ->
            "ln(#{@expr})"
    
    Derivative: class
        constructor: (@expr) ->
        derive: ->
            @simplify().derive()
        simplify: ->
            @expr.derive().simplify()
        identical: ->
            other instanceof AST.Derivative and @expr.identical(other.expr)
        toString: ->
            "d/dx #{@expr}"
            

# UI:

print = (s) -> OS.write OS.stdout, s
puts = (s) -> print "#{s}\n"
gets = (fn) -> OS.read OS.stdin, 0, (err, buff) -> fn buff

evaluate = (str) -> new Parser(lex str.trim()).parse().simplify()

if OS.argv.length > 1
    puts evaluate OS.argv.slice(1).join " "
    do OS.exit
else
    puts "JSOS Calc"
    do prompt = ->
        print ">> "
        gets (str) ->
            str = str.trim()
            if str is "q" or str is "quit" or str is "exit"
                do OS.exit
            else
                try
                    puts "=> #{evaluate str}"
                catch e
                    throw e unless e instanceof Parser.Error
                    puts "Syntax Error: #{e.message}"
                do prompt