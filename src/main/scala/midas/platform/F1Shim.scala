package midas
package platform

import util.ParameterizedBundle // from rocketchip

import chisel3._
import chisel3.util._
import junctions._
import config.{Parameters, Field}

class F1ShimIO(implicit p: Parameters) extends ParameterizedBundle()(p) {
  val master = Flipped(new NastiIO()(p alterPartial ({ case NastiKey => p(MasterNastiKey) })))
  val slave  = new NastiIO()(p alterPartial ({ case NastiKey => p(SlaveNastiKey) }))
}

class F1Shim(simIo: midas.core.SimWrapperIO)
              (implicit p: Parameters) extends PlatformShim {
  val io = IO(new F1ShimIO)
  val top = Module(new midas.core.FPGATop(simIo))
  val headerConsts = List(
    "MMIO_WIDTH" -> p(MasterNastiKey).dataBits / 8,
    "MEM_WIDTH"  -> p(SlaveNastiKey).dataBits / 8
  ) ++ top.headerConsts


  when (io.master.aw.fire()) {
    printf("[master,awfire] addr %x, len %x, size %x, burst %x, lock %x, cache %x, prot %x, qos %x, region %x, id %x, user %x\n",
      io.master.aw.bits.addr,
      io.master.aw.bits.len,
      io.master.aw.bits.size,
      io.master.aw.bits.burst,
      io.master.aw.bits.lock,
      io.master.aw.bits.cache,
      io.master.aw.bits.prot,
      io.master.aw.bits.qos,
      io.master.aw.bits.region,
      io.master.aw.bits.id,
      io.master.aw.bits.user
      )
  }

  when (io.master.w.fire()) {
    printf("[master,wfire] data %x, last %x, id %x, strb %x, user %x\n",
      io.master.w.bits.data,
      io.master.w.bits.last,
      io.master.w.bits.id,
      io.master.w.bits.strb,
      io.master.w.bits.user
      )
  }

  when (io.master.b.fire()) {
    printf("[master,bfire] resp %x, id %x, user %x\n",
      io.master.b.bits.resp,
      io.master.b.bits.id,
      io.master.b.bits.user
      )
  }

  when (io.master.ar.fire()) {
    printf("[master,arfire] addr %x, len %x, size %x, burst %x, lock %x, cache %x, prot %x, qos %x, region %x, id %x, user %x\n",
      io.master.ar.bits.addr,
      io.master.ar.bits.len,
      io.master.ar.bits.size,
      io.master.ar.bits.burst,
      io.master.ar.bits.lock,
      io.master.ar.bits.cache,
      io.master.ar.bits.prot,
      io.master.ar.bits.qos,
      io.master.ar.bits.region,
      io.master.ar.bits.id,
      io.master.ar.bits.user
      )
  }

  when (io.master.r.fire()) {
    printf("[master,rfire] resp %x, data %x, last %x, id %x, user %x\n",
      io.master.r.bits.resp,
      io.master.r.bits.data,
      io.master.r.bits.last,
      io.master.r.bits.id,
      io.master.r.bits.user
      )
  }

  when (io.slave.aw.fire()) {
    printf("[slave,awfire] addr %x, len %x, size %x, burst %x, lock %x, cache %x, prot %x, qos %x, region %x, id %x, user %x\n",
      io.slave.aw.bits.addr,
      io.slave.aw.bits.len,
      io.slave.aw.bits.size,
      io.slave.aw.bits.burst,
      io.slave.aw.bits.lock,
      io.slave.aw.bits.cache,
      io.slave.aw.bits.prot,
      io.slave.aw.bits.qos,
      io.slave.aw.bits.region,
      io.slave.aw.bits.id,
      io.slave.aw.bits.user
      )
  }

  when (io.slave.w.fire()) {
    printf("[slave,wfire] data %x, last %x, id %x, strb %x, user %x\n",
      io.slave.w.bits.data,
      io.slave.w.bits.last,
      io.slave.w.bits.id,
      io.slave.w.bits.strb,
      io.slave.w.bits.user
      )
  }

  when (io.slave.b.fire()) {
    printf("[slave,bfire] resp %x, id %x, user %x\n",
      io.slave.b.bits.resp,
      io.slave.b.bits.id,
      io.slave.b.bits.user
      )
  }

  when (io.slave.ar.fire()) {
    printf("[slave,arfire] addr %x, len %x, size %x, burst %x, lock %x, cache %x, prot %x, qos %x, region %x, id %x, user %x\n",
      io.slave.ar.bits.addr,
      io.slave.ar.bits.len,
      io.slave.ar.bits.size,
      io.slave.ar.bits.burst,
      io.slave.ar.bits.lock,
      io.slave.ar.bits.cache,
      io.slave.ar.bits.prot,
      io.slave.ar.bits.qos,
      io.slave.ar.bits.region,
      io.slave.ar.bits.id,
      io.slave.ar.bits.user
      )
  }

  when (io.slave.r.fire()) {
    printf("[slave,rfire] resp %x, data %x, last %x, id %x, user %x\n",
      io.slave.r.bits.resp,
      io.slave.r.bits.data,
      io.slave.r.bits.last,
      io.slave.r.bits.id,
      io.slave.r.bits.user
      )
  }



  top.io.ctrl <> io.master
  io.slave <> top.io.mem
}
