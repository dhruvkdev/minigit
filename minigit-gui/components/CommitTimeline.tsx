"use client";

import { motion, AnimatePresence } from "framer-motion";
import { GitCommitHorizontal, Clock, Calendar, Hash, ChevronRight } from "lucide-react";
import { useState } from "react";

interface Commit {
  id: string;
  message: string;
  date: string;
  time: string;
  isHead: boolean;
}

interface CommitTimelineProps {
  commits: Commit[];
  loading?: boolean;
}

function CommitSkeleton() {
  return (
    <div className="flex gap-4 items-start py-4">
      <div className="flex flex-col items-center" style={{ minWidth: 40 }}>
        <div className="skeleton w-8 h-8 rounded-full" />
        <div className="w-[2px] flex-1 mt-2" style={{ background: "var(--border-subtle)", minHeight: 50 }} />
      </div>
      <div className="flex-1 space-y-2 pb-4">
        <div className="skeleton h-4 w-3/4" />
        <div className="skeleton h-3 w-1/3" />
        <div className="skeleton h-3 w-1/2" />
      </div>
    </div>
  );
}

export function CommitTimeline({ commits, loading }: CommitTimelineProps) {
  const [expandedId, setExpandedId] = useState<string | null>(null);

  if (loading) {
    return (
      <div className="glass-card p-5">
        <h2 className="text-sm font-semibold mb-4" style={{ color: "var(--text-secondary)" }}>
          COMMIT HISTORY
        </h2>
        {[1, 2, 3].map(i => <CommitSkeleton key={i} />)}
      </div>
    );
  }

  if (commits.length === 0) {
    return (
      <div className="glass-card p-5 flex flex-col items-center justify-center gap-3" style={{ minHeight: 200 }}>
        <GitCommitHorizontal size={32} style={{ color: "var(--text-muted)" }} />
        <p className="text-sm" style={{ color: "var(--text-muted)" }}>No commits yet</p>
      </div>
    );
  }

  return (
    <div className="glass-card p-5">
      <h2 className="text-sm font-semibold tracking-wider mb-4" style={{ color: "var(--text-secondary)" }}>
        COMMIT HISTORY
      </h2>
      <div className="relative">
        {commits.map((commit, idx) => (
          <motion.div
            key={commit.id}
            initial={{ opacity: 0, x: -16 }}
            animate={{ opacity: 1, x: 0 }}
            transition={{ delay: idx * 0.06, duration: 0.35 }}
            className="flex gap-4 items-start"
          >
            {/* Left timeline */}
            <div className="flex flex-col items-center" style={{ minWidth: 40 }}>
              <motion.div
                whileHover={{ scale: 1.3 }}
                onClick={() => setExpandedId(expandedId === commit.id ? null : commit.id)}
                className="w-8 h-8 rounded-full flex items-center justify-center cursor-pointer z-10 relative"
                style={{
                  background: commit.isHead
                    ? "linear-gradient(135deg, #6366f1, #818cf8)"
                    : "rgba(30,36,58,1)",
                  border: `2px solid ${commit.isHead ? "#6366f1" : "var(--border-subtle)"}`,
                  boxShadow: commit.isHead ? "0 0 14px rgba(99,102,241,0.6)" : "none",
                }}
              >
                <GitCommitHorizontal size={14} color={commit.isHead ? "#fff" : "var(--text-muted)"} />
              </motion.div>
              {idx < commits.length - 1 && (
                <div
                  style={{
                    width: 2,
                    flex: 1,
                    minHeight: 40,
                    background: "linear-gradient(to bottom, var(--border-bright), var(--border-subtle))",
                    marginTop: 4,
                  }}
                />
              )}
            </div>

            {/* Commit card */}
            <div className="flex-1 pb-5">
              <div
                className="rounded-xl p-4 cursor-pointer transition-all duration-200"
                style={{
                  background: commit.isHead
                    ? "rgba(99,102,241,0.08)"
                    : "rgba(15,18,32,0.5)",
                  border: `1px solid ${commit.isHead ? "var(--border-bright)" : "var(--border-subtle)"}`,
                }}
                onClick={() => setExpandedId(expandedId === commit.id ? null : commit.id)}
              >
                <div className="flex items-start justify-between gap-2">
                  <div className="flex flex-col gap-1 flex-1 min-w-0">
                    <div className="flex items-center gap-2 flex-wrap">
                      {commit.isHead && (
                        <span className="badge badge-indigo text-[10px]">HEAD</span>
                      )}
                      <p
                        className="text-sm font-semibold truncate"
                        style={{ color: "var(--text-primary)" }}
                      >
                        {commit.message}
                      </p>
                    </div>
                    <div className="flex items-center gap-3 flex-wrap mt-1">
                      <span
                        className="mono text-[11px] flex items-center gap-1"
                        style={{ color: "var(--accent-secondary)" }}
                      >
                        <Hash size={11} />
                        {commit.id.substring(0, 12)}
                      </span>
                      <span
                        className="text-[11px] flex items-center gap-1"
                        style={{ color: "var(--text-muted)" }}
                      >
                        <Calendar size={11} /> {commit.date}
                      </span>
                      <span
                        className="text-[11px] flex items-center gap-1"
                        style={{ color: "var(--text-muted)" }}
                      >
                        <Clock size={11} /> {commit.time}
                      </span>
                    </div>
                  </div>
                  <motion.div
                    animate={{ rotate: expandedId === commit.id ? 90 : 0 }}
                    transition={{ duration: 0.2 }}
                  >
                    <ChevronRight size={16} style={{ color: "var(--text-muted)" }} />
                  </motion.div>
                </div>

                <AnimatePresence>
                  {expandedId === commit.id && (
                    <motion.div
                      initial={{ height: 0, opacity: 0 }}
                      animate={{ height: "auto", opacity: 1 }}
                      exit={{ height: 0, opacity: 0 }}
                      transition={{ duration: 0.25 }}
                      style={{ overflow: "hidden" }}
                    >
                      <div
                        className="mt-3 pt-3 mono text-[11px]"
                        style={{
                          borderTop: "1px solid var(--border-subtle)",
                          color: "var(--text-secondary)",
                        }}
                      >
                        Full Hash: {commit.id}
                      </div>
                    </motion.div>
                  )}
                </AnimatePresence>
              </div>
            </div>
          </motion.div>
        ))}
      </div>
    </div>
  );
}
