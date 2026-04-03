"use client";

import { motion } from "framer-motion";
import { GitBranch, Hash, Calendar, Clock, RefreshCcw, Loader2 } from "lucide-react";

interface RepoStatusProps {
  currentBranch: string;
  latestCommit: string;
  latestCommitDate: string;
  latestCommitTime: string;
  onRefresh: () => void;
  refreshing?: boolean;
}

export function RepoStatus({
  currentBranch,
  latestCommit,
  latestCommitDate,
  latestCommitTime,
  onRefresh,
  refreshing,
}: RepoStatusProps) {
  const stats = [
    {
      icon: GitBranch,
      label: "Current Branch",
      value: currentBranch || "main",
      badge: "badge-indigo",
    },
    {
      icon: Hash,
      label: "Latest Commit",
      value: latestCommit ? latestCommit.substring(0, 14) : "No commits yet",
      mono: true,
      badge: latestCommit ? "badge-green" : "badge-amber",
    },
    {
      icon: Calendar,
      label: "Commit Date",
      value: latestCommitDate || "—",
    },
    {
      icon: Clock,
      label: "Commit Time",
      value: latestCommitTime || "—",
    },
  ];

  return (
    <div className="glass-card p-5">
      <div className="flex items-center justify-between mb-5">
        <div>
          <h1 className="text-xl font-bold gradient-text">minigit</h1>
          <p className="text-xs mt-0.5" style={{ color: "var(--text-muted)" }}>
            Repository Dashboard
          </p>
        </div>
        <div className="flex items-center gap-3">
          <div className="pulse-dot" />
          <motion.button
            className="btn-ghost p-2 rounded-lg"
            onClick={onRefresh}
            disabled={refreshing}
            whileTap={{ scale: 0.9 }}
          >
            {refreshing ? (
              <Loader2 size={15} className="animate-spin" style={{ color: "var(--accent-secondary)" }} />
            ) : (
              <RefreshCcw size={15} style={{ color: "var(--text-secondary)" }} />
            )}
          </motion.button>
        </div>
      </div>

      <div className="grid grid-cols-2 gap-3">
        {stats.map((stat, idx) => {
          const Icon = stat.icon;
          return (
            <motion.div
              key={stat.label}
              initial={{ opacity: 0, y: 12 }}
              animate={{ opacity: 1, y: 0 }}
              transition={{ delay: idx * 0.08 }}
              className="rounded-xl p-3 flex flex-col gap-1.5"
              style={{
                background: "rgba(15,18,32,0.6)",
                border: "1px solid var(--border-subtle)",
              }}
            >
              <div className="flex items-center gap-1.5">
                <Icon size={12} style={{ color: "var(--text-muted)" }} />
                <span className="text-[11px] font-medium tracking-wide" style={{ color: "var(--text-muted)" }}>
                  {stat.label}
                </span>
              </div>
              <p
                className={`text-sm font-semibold truncate ${stat.mono ? "mono" : ""}`}
                style={{ color: "var(--text-primary)" }}
              >
                {stat.value}
              </p>
            </motion.div>
          );
        })}
      </div>
    </div>
  );
}
