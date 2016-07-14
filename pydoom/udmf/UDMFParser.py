# Generated from UDMF.g4 by ANTLR 4.5.3
# encoding: utf-8
from __future__ import print_function
from antlr4 import *
from io import StringIO

def serializedATN():
    with StringIO() as buf:
        buf.write(u"\3\u0430\ud6d1\u8206\uad2d\u4417\uaef1\u8d80\uaadd\3")
        buf.write(u"\17(\4\2\t\2\4\3\t\3\4\4\t\4\4\5\t\5\4\6\t\6\3\2\7\2")
        buf.write(u"\16\n\2\f\2\16\2\21\13\2\3\3\3\3\5\3\25\n\3\3\4\3\4\3")
        buf.write(u"\4\7\4\32\n\4\f\4\16\4\35\13\4\3\4\3\4\3\5\3\5\3\5\3")
        buf.write(u"\5\3\5\3\6\3\6\3\6\2\2\7\2\4\6\b\n\2\3\4\2\7\7\t\f%\2")
        buf.write(u"\17\3\2\2\2\4\24\3\2\2\2\6\26\3\2\2\2\b \3\2\2\2\n%\3")
        buf.write(u"\2\2\2\f\16\5\4\3\2\r\f\3\2\2\2\16\21\3\2\2\2\17\r\3")
        buf.write(u"\2\2\2\17\20\3\2\2\2\20\3\3\2\2\2\21\17\3\2\2\2\22\25")
        buf.write(u"\5\6\4\2\23\25\5\b\5\2\24\22\3\2\2\2\24\23\3\2\2\2\25")
        buf.write(u"\5\3\2\2\2\26\27\7\b\2\2\27\33\7\3\2\2\30\32\5\b\5\2")
        buf.write(u"\31\30\3\2\2\2\32\35\3\2\2\2\33\31\3\2\2\2\33\34\3\2")
        buf.write(u"\2\2\34\36\3\2\2\2\35\33\3\2\2\2\36\37\7\4\2\2\37\7\3")
        buf.write(u"\2\2\2 !\7\b\2\2!\"\7\5\2\2\"#\5\n\6\2#$\7\6\2\2$\t\3")
        buf.write(u"\2\2\2%&\t\2\2\2&\13\3\2\2\2\5\17\24\33")
        return buf.getvalue()


class UDMFParser ( Parser ):

    grammarFileName = "UDMF.g4"

    atn = ATNDeserializer().deserialize(serializedATN())

    decisionsToDFA = [ DFA(ds, i) for i, ds in enumerate(atn.decisionToState) ]

    sharedContextCache = PredictionContextCache()

    literalNames = [ u"<INVALID>", u"'{'", u"'}'", u"'='", u"';'" ]

    symbolicNames = [ u"<INVALID>", u"<INVALID>", u"<INVALID>", u"<INVALID>", 
                      u"<INVALID>", u"Boolean", u"Identifier", u"Integer", 
                      u"Float", u"QuotedString", u"Keyword", u"WhiteSpace", 
                      u"BlockComment", u"LineComment" ]

    RULE_udmf = 0
    RULE_globalExpression = 1
    RULE_blockExpression = 2
    RULE_assignmentExpression = 3
    RULE_value = 4

    ruleNames =  [ u"udmf", u"globalExpression", u"blockExpression", u"assignmentExpression", 
                   u"value" ]

    EOF = Token.EOF
    T__0=1
    T__1=2
    T__2=3
    T__3=4
    Boolean=5
    Identifier=6
    Integer=7
    Float=8
    QuotedString=9
    Keyword=10
    WhiteSpace=11
    BlockComment=12
    LineComment=13

    def __init__(self, input):
        super(UDMFParser, self).__init__(input)
        self.checkVersion("4.5.3")
        self._interp = ParserATNSimulator(self, self.atn, self.decisionsToDFA, self.sharedContextCache)
        self._predicates = None



    class UdmfContext(ParserRuleContext):

        def __init__(self, parser, parent=None, invokingState=-1):
            super(UDMFParser.UdmfContext, self).__init__(parent, invokingState)
            self.parser = parser

        def globalExpression(self, i=None):
            if i is None:
                return self.getTypedRuleContexts(UDMFParser.GlobalExpressionContext)
            else:
                return self.getTypedRuleContext(UDMFParser.GlobalExpressionContext,i)


        def getRuleIndex(self):
            return UDMFParser.RULE_udmf

        def enterRule(self, listener):
            if hasattr(listener, "enterUdmf"):
                listener.enterUdmf(self)

        def exitRule(self, listener):
            if hasattr(listener, "exitUdmf"):
                listener.exitUdmf(self)




    def udmf(self):

        localctx = UDMFParser.UdmfContext(self, self._ctx, self.state)
        self.enterRule(localctx, 0, self.RULE_udmf)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 13
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while _la==UDMFParser.Identifier:
                self.state = 10
                self.globalExpression()
                self.state = 15
                self._errHandler.sync(self)
                _la = self._input.LA(1)

        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class GlobalExpressionContext(ParserRuleContext):

        def __init__(self, parser, parent=None, invokingState=-1):
            super(UDMFParser.GlobalExpressionContext, self).__init__(parent, invokingState)
            self.parser = parser

        def blockExpression(self):
            return self.getTypedRuleContext(UDMFParser.BlockExpressionContext,0)


        def assignmentExpression(self):
            return self.getTypedRuleContext(UDMFParser.AssignmentExpressionContext,0)


        def getRuleIndex(self):
            return UDMFParser.RULE_globalExpression

        def enterRule(self, listener):
            if hasattr(listener, "enterGlobalExpression"):
                listener.enterGlobalExpression(self)

        def exitRule(self, listener):
            if hasattr(listener, "exitGlobalExpression"):
                listener.exitGlobalExpression(self)




    def globalExpression(self):

        localctx = UDMFParser.GlobalExpressionContext(self, self._ctx, self.state)
        self.enterRule(localctx, 2, self.RULE_globalExpression)
        try:
            self.state = 18
            self._errHandler.sync(self);
            la_ = self._interp.adaptivePredict(self._input,1,self._ctx)
            if la_ == 1:
                self.enterOuterAlt(localctx, 1)
                self.state = 16
                self.blockExpression()
                pass

            elif la_ == 2:
                self.enterOuterAlt(localctx, 2)
                self.state = 17
                self.assignmentExpression()
                pass


        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class BlockExpressionContext(ParserRuleContext):

        def __init__(self, parser, parent=None, invokingState=-1):
            super(UDMFParser.BlockExpressionContext, self).__init__(parent, invokingState)
            self.parser = parser

        def Identifier(self):
            return self.getToken(UDMFParser.Identifier, 0)

        def assignmentExpression(self, i=None):
            if i is None:
                return self.getTypedRuleContexts(UDMFParser.AssignmentExpressionContext)
            else:
                return self.getTypedRuleContext(UDMFParser.AssignmentExpressionContext,i)


        def getRuleIndex(self):
            return UDMFParser.RULE_blockExpression

        def enterRule(self, listener):
            if hasattr(listener, "enterBlockExpression"):
                listener.enterBlockExpression(self)

        def exitRule(self, listener):
            if hasattr(listener, "exitBlockExpression"):
                listener.exitBlockExpression(self)




    def blockExpression(self):

        localctx = UDMFParser.BlockExpressionContext(self, self._ctx, self.state)
        self.enterRule(localctx, 4, self.RULE_blockExpression)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 20
            self.match(UDMFParser.Identifier)
            self.state = 21
            self.match(UDMFParser.T__0)
            self.state = 25
            self._errHandler.sync(self)
            _la = self._input.LA(1)
            while _la==UDMFParser.Identifier:
                self.state = 22
                self.assignmentExpression()
                self.state = 27
                self._errHandler.sync(self)
                _la = self._input.LA(1)

            self.state = 28
            self.match(UDMFParser.T__1)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class AssignmentExpressionContext(ParserRuleContext):

        def __init__(self, parser, parent=None, invokingState=-1):
            super(UDMFParser.AssignmentExpressionContext, self).__init__(parent, invokingState)
            self.parser = parser

        def Identifier(self):
            return self.getToken(UDMFParser.Identifier, 0)

        def value(self):
            return self.getTypedRuleContext(UDMFParser.ValueContext,0)


        def getRuleIndex(self):
            return UDMFParser.RULE_assignmentExpression

        def enterRule(self, listener):
            if hasattr(listener, "enterAssignmentExpression"):
                listener.enterAssignmentExpression(self)

        def exitRule(self, listener):
            if hasattr(listener, "exitAssignmentExpression"):
                listener.exitAssignmentExpression(self)




    def assignmentExpression(self):

        localctx = UDMFParser.AssignmentExpressionContext(self, self._ctx, self.state)
        self.enterRule(localctx, 6, self.RULE_assignmentExpression)
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 30
            self.match(UDMFParser.Identifier)
            self.state = 31
            self.match(UDMFParser.T__2)
            self.state = 32
            self.value()
            self.state = 33
            self.match(UDMFParser.T__3)
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx

    class ValueContext(ParserRuleContext):

        def __init__(self, parser, parent=None, invokingState=-1):
            super(UDMFParser.ValueContext, self).__init__(parent, invokingState)
            self.parser = parser

        def Boolean(self):
            return self.getToken(UDMFParser.Boolean, 0)

        def Integer(self):
            return self.getToken(UDMFParser.Integer, 0)

        def Float(self):
            return self.getToken(UDMFParser.Float, 0)

        def QuotedString(self):
            return self.getToken(UDMFParser.QuotedString, 0)

        def Keyword(self):
            return self.getToken(UDMFParser.Keyword, 0)

        def getRuleIndex(self):
            return UDMFParser.RULE_value

        def enterRule(self, listener):
            if hasattr(listener, "enterValue"):
                listener.enterValue(self)

        def exitRule(self, listener):
            if hasattr(listener, "exitValue"):
                listener.exitValue(self)




    def value(self):

        localctx = UDMFParser.ValueContext(self, self._ctx, self.state)
        self.enterRule(localctx, 8, self.RULE_value)
        self._la = 0 # Token type
        try:
            self.enterOuterAlt(localctx, 1)
            self.state = 35
            _la = self._input.LA(1)
            if not((((_la) & ~0x3f) == 0 and ((1 << _la) & ((1 << UDMFParser.Boolean) | (1 << UDMFParser.Integer) | (1 << UDMFParser.Float) | (1 << UDMFParser.QuotedString) | (1 << UDMFParser.Keyword))) != 0)):
                self._errHandler.recoverInline(self)
            else:
                self.consume()
        except RecognitionException as re:
            localctx.exception = re
            self._errHandler.reportError(self, re)
            self._errHandler.recover(self, re)
        finally:
            self.exitRule()
        return localctx





