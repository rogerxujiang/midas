package strober
package passes

import firrtl._
import firrtl.ir._
import firrtl.Mappers._
import firrtl.passes.MemPortUtils.memPortField
import midas.passes.Utils._

class SeqMemToRegFile extends firrtl.passes.Pass {
  override def inputForm = LowForm
  override def outputForm = LowForm

  type Netlist = scala.collection.mutable.HashMap[String, Expression]
  type Statements = scala.collection.mutable.ArrayBuffer[Statement]

  private def buildNetlist(netlist: Netlist)(s: Statement): Statement = {
    s match {
      case s: Connect => firrtl.Utils.kind(s.loc) match {
        case MemKind => netlist(s.loc.serialize) = s.expr
        case _ =>
      }
      case _ =>
    }
    s map buildNetlist(netlist)
  }

  private def seqMemToRegFile(netlist: Netlist, repl: Netlist, stmts: Statements)(s: Statement): Statement =
    s map seqMemToRegFile(netlist, repl, stmts) match {
      case s: DefMemory if s.readLatency == 1 => Block(
        Seq(s.copy(readLatency = 0)) ++
        // FIXME: handle readers and readwriters in the same way
        (s.readers map { r =>
          val clock = netlist(memPortField(s, r, "clk").serialize)
          val enable = netlist(memPortField(s, r, "en").serialize)
          val address = memPortField(s, r, "addr")
          val tpe = UIntType(IntWidth(firrtl.Utils.ceilLog2(s.depth)))
          val reg = WRef(s"${s.name}_${r}_addr_reg", tpe)
          repl(address.serialize) = reg
          stmts += Connect(s.info, reg, Mux(enable, netlist(address.serialize), reg, tpe))
          stmts += Connect(s.info, memPortField(s, r, "addr"), reg)
          DefRegister(s.info, reg.name, s.dataType, clock, firrtl.Utils.zero, reg)
        }) ++
        (s.readwriters map { rw =>
          val clock = netlist(memPortField(s, rw, "clk").serialize)
          val enable = netlist(memPortField(s, rw, "en").serialize)
          val wmode = netlist(memPortField(s, rw, "wmode").serialize)
          val data = memPortField(s, rw, "rdata")
          val reg = WRef(s"${s.name}_${rw}_rdata_reg", s.dataType)
          repl(data.serialize) = reg
          stmts += Connect(s.info, reg, Mux(and(enable, not(wmode)), data, reg, s.dataType))
          DefRegister(s.info, reg.name, s.dataType, clock, firrtl.Utils.zero, reg)
        })
      )
      case s => s
    }

  private def replaceOnExp(repl: Netlist)(e: Expression): Expression =
    e match {
      case _: WSubField => repl get e.serialize match {
        case None => e
        case Some(ex) => ex
      }
      case _ => e map replaceOnExp(repl)
    }

  private def replaceOnStmt(repl: Netlist)(s: Statement): Statement =
    s map replaceOnStmt(repl) match {
      case s: Connect =>
        s.copy(loc = replaceOnExp(repl)(s.loc))
      case s => s
    }

  private def onMod(m: DefModule): DefModule = {
    val (netlist, repl, stmts) = (new Netlist, new Netlist, new Statements)
    (m map buildNetlist(netlist)
       map seqMemToRegFile(netlist, repl, stmts)
       map replaceOnStmt(repl)) match {
      case m: ExtModule => m
      case m: Module => m.copy(body = Block(m.body +: stmts.toSeq))
    }
  }

  def run(c: Circuit) = c.copy(modules = c.modules map onMod)
}
