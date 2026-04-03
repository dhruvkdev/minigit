"use client";

import { useCallback, useEffect, useState } from "react";
import { motion } from "framer-motion";
import { RepoStatus } from "@/components/RepoStatus";
import { CommitTimeline } from "@/components/CommitTimeline";
import { StagingArea } from "@/components/StagingArea";
import { BranchManager } from "@/components/BranchManager";
import { CommitPanel } from "@/components/CommitPanel";
import { Terminal, GitMerge, LayoutDashboard } from "lucide-react";

interface Commit {
  id: string;
  message: string;
  date: string;
  time: string;
  isHead: boolean;
}

interface StatusData {
  currentBranch: string;
  latestCommit: string;
  latestCommitDate: string;
  latestCommitTime: string;
  stagedFiles: string[];
  branches: string[];
  error?: string;
}

export default function Dashboard() {
  const [status, setStatus] = useState<StatusData>({
    currentBranch: "main",
    latestCommit: "",
    latestCommitDate: "",
    latestCommitTime: "",
    stagedFiles: [],
    branches: [],
  });
  const [commits, setCommits] = useState<Commit[]>([]);
  const [loadingStatus, setLoadingStatus] = useState(true);
  const [loadingLog, setLoadingLog] = useState(true);
  const [refreshing, setRefreshing] = useState(false);

  const fetchStatus = useCallback(async () => {
    try {
      const res = await fetch("/api/status");
      const data: StatusData = await res.json();
      setStatus(data);
    } catch {
      // keep defaults
    }
  }, []);

  const fetchLog = useCallback(async () => {
    try {
      const res = await fetch("/api/log");
      const data: { commits: Commit[] } = await res.json();
      setCommits(data.commits || []);
    } catch {
      setCommits([]);
    }
  }, []);

  const refreshAll = useCallback(async () => {
    setRefreshing(true);
    await Promise.all([fetchStatus(), fetchLog()]);
    setRefreshing(false);
  }, [fetchStatus, fetchLog]);

  useEffect(() => {
    (async () => {
      await Promise.all([fetchStatus(), fetchLog()]);
      setLoadingStatus(false);
      setLoadingLog(false);
    })();
  }, [fetchStatus, fetchLog]);

  const handleStageAll = async () => {
    const res = await fetch("/api/add", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ files: ["."] }),
    });
    return res.json();
  };

  const handleCommit = async (message: string) => {
    const res = await fetch("/api/commit", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ message }),
    });
    return res.json();
  };

  const handleBranchAction = async (action: "create" | "checkout", name: string) => {
    await fetch("/api/branch", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ action, name }),
    });
  };

  return (
    <div className="relative min-h-screen" style={{ background: "var(--bg-primary)" }}>
      {/* Ambient background orbs */}
      <div className="orb" style={{ width: 600, height: 600, background: "#6366f1", top: -200, left: -200 }} />
      <div className="orb" style={{ width: 500, height: 500, background: "#22d3ee", bottom: -150, right: -100 }} />
      <div className="orb" style={{ width: 300, height: 300, background: "#818cf8", top: "50%", left: "40%" }} />

      {/* Top navbar */}
      <header
        className="sticky top-0 z-50 flex items-center justify-between px-6 py-3"
        style={{
          background: "rgba(10,12,20,0.85)",
          borderBottom: "1px solid var(--border-subtle)",
          backdropFilter: "blur(20px)",
        }}
      >
        <div className="flex items-center gap-3">
          <div
            className="w-8 h-8 rounded-xl flex items-center justify-center"
            style={{
              background: "linear-gradient(135deg, #6366f1, #818cf8)",
              boxShadow: "0 0 20px rgba(99,102,241,0.5)",
            }}
          >
            <GitMerge size={16} color="#fff" />
          </div>
          <span className="font-bold text-lg gradient-text">minigit</span>
          <span
            className="text-xs px-2 py-0.5 rounded-full"
            style={{
              background: "rgba(99,102,241,0.12)",
              color: "var(--accent-secondary)",
              border: "1px solid var(--border-bright)",
            }}
          >
            GUI Dashboard
          </span>
        </div>

        <div className="flex items-center gap-4">
          <div className="flex items-center gap-2">
            <span className="w-2 h-2 rounded-full" style={{ background: "var(--accent-green)" }} />
            <span className="text-xs font-medium" style={{ color: "var(--text-secondary)" }}>
              {status.currentBranch}
            </span>
          </div>
          <a
            href="https://github.com"
            className="flex items-center gap-1.5 text-xs btn-ghost px-3 py-1.5"
            style={{ color: "var(--text-secondary)" }}
          >
            <Terminal size={12} />
            Terminal
          </a>
        </div>
      </header>

      {/* Main grid */}
      <main className="relative z-10 max-w-7xl mx-auto px-6 py-8">
        {/* Page title */}
        <motion.div
          initial={{ opacity: 0, y: -12 }}
          animate={{ opacity: 1, y: 0 }}
          className="mb-7 flex items-center gap-3"
        >
          <LayoutDashboard size={18} style={{ color: "var(--accent-secondary)" }} />
          <h1 className="text-base font-semibold" style={{ color: "var(--text-primary)" }}>
            Repository Overview
          </h1>
          <span className="text-xs" style={{ color: "var(--text-muted)" }}>
            — powered by minigit
          </span>
        </motion.div>

        {/* 3-column layout */}
        <div className="grid grid-cols-1 lg:grid-cols-3 gap-6 items-start">
          {/* Left column */}
          <div className="flex flex-col gap-5">
            <motion.div
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              transition={{ duration: 0.4 }}
            >
              {loadingStatus ? (
                <div className="glass-card p-5 space-y-4">
                  <div className="skeleton h-5 w-1/2" />
                  <div className="grid grid-cols-2 gap-3">
                    {[1, 2, 3, 4].map(i => <div key={i} className="skeleton h-16 rounded-xl" />)}
                  </div>
                </div>
              ) : (
                <RepoStatus
                  currentBranch={status.currentBranch}
                  latestCommit={status.latestCommit}
                  latestCommitDate={status.latestCommitDate}
                  latestCommitTime={status.latestCommitTime}
                  onRefresh={refreshAll}
                  refreshing={refreshing}
                />
              )}
            </motion.div>

            <motion.div
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              transition={{ duration: 0.4, delay: 0.1 }}
            >
              <BranchManager
                currentBranch={status.currentBranch}
                branches={status.branches}
                onBranchAction={handleBranchAction}
                onRefresh={refreshAll}
              />
            </motion.div>
          </div>

          {/* Middle column — Commit History (takes largest space) */}
          <motion.div
            initial={{ opacity: 0, y: 20 }}
            animate={{ opacity: 1, y: 0 }}
            transition={{ duration: 0.4, delay: 0.05 }}
            className="lg:col-span-1"
          >
            <CommitTimeline commits={commits} loading={loadingLog} />
          </motion.div>

          {/* Right column */}
          <div className="flex flex-col gap-5">
            <motion.div
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              transition={{ duration: 0.4, delay: 0.15 }}
            >
              <StagingArea
                stagedFiles={status.stagedFiles}
                onStageAll={handleStageAll}
                onRefresh={refreshAll}
              />
            </motion.div>

            <motion.div
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              transition={{ duration: 0.4, delay: 0.2 }}
            >
              <CommitPanel onCommit={handleCommit} onRefresh={refreshAll} />
            </motion.div>
          </div>
        </div>

        {/* Footer */}
        <motion.footer
          initial={{ opacity: 0 }}
          animate={{ opacity: 1 }}
          transition={{ delay: 0.6 }}
          className="mt-12 pt-5 flex items-center justify-between text-xs"
          style={{
            borderTop: "1px solid var(--border-subtle)",
            color: "var(--text-muted)",
          }}
        >
          <span>minigit GUI — built as an OOPs project enhancement</span>
          <span className="mono">C++23 · Next.js · Tailwind</span>
        </motion.footer>
      </main>
    </div>
  );
}
