"use client";

import { motion, AnimatePresence } from "framer-motion";
import { GitBranch, Plus, ArrowRight, Check, Loader2, ChevronDown } from "lucide-react";
import { useState } from "react";

interface BranchManagerProps {
  currentBranch: string;
  branches: string[];
  onBranchAction: (action: "create" | "checkout", name: string) => Promise<void>;
  onRefresh: () => void;
}

export function BranchManager({ currentBranch, branches, onBranchAction, onRefresh }: BranchManagerProps) {
  const [newBranchName, setNewBranchName] = useState("");
  const [creating, setCreating] = useState(false);
  const [createSuccess, setCreateSuccess] = useState(false);
  const [checkingOut, setCheckingOut] = useState<string | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [showCreate, setShowCreate] = useState(false);

  const handleCreate = async () => {
    if (!newBranchName.trim()) return;
    setCreating(true);
    setError(null);
    try {
      await onBranchAction("create", newBranchName.trim());
      setCreateSuccess(true);
      setNewBranchName("");
      onRefresh();
      setTimeout(() => { setCreateSuccess(false); setShowCreate(false); }, 2000);
    } catch {
      setError("Failed to create branch.");
    } finally {
      setCreating(false);
    }
  };

  const handleCheckout = async (branch: string) => {
    if (branch === currentBranch) return;
    setCheckingOut(branch);
    setError(null);
    try {
      await onBranchAction("checkout", branch);
      onRefresh();
    } catch {
      setError("Checkout failed.");
    } finally {
      setCheckingOut(null);
    }
  };

  return (
    <div className="glass-card p-5">
      <div className="flex items-center justify-between mb-4">
        <h2 className="text-sm font-semibold tracking-wider" style={{ color: "var(--text-secondary)" }}>
          BRANCHES
        </h2>
        <motion.button
          className="btn-ghost flex items-center gap-1 px-3 py-2"
          onClick={() => setShowCreate(v => !v)}
          whileTap={{ scale: 0.97 }}
        >
          <Plus size={12} />
          New
          <motion.div animate={{ rotate: showCreate ? 180 : 0 }} transition={{ duration: 0.2 }}>
            <ChevronDown size={12} />
          </motion.div>
        </motion.button>
      </div>

      {/* Create branch input */}
      <AnimatePresence>
        {showCreate && (
          <motion.div
            initial={{ height: 0, opacity: 0 }}
            animate={{ height: "auto", opacity: 1 }}
            exit={{ height: 0, opacity: 0 }}
            transition={{ duration: 0.2 }}
            style={{ overflow: "hidden" }}
          >
            <div className="flex gap-2 mb-4">
              <input
                className="input-field px-3 py-2"
                placeholder="branch-name"
                value={newBranchName}
                onChange={e => setNewBranchName(e.target.value)}
                onKeyDown={e => e.key === "Enter" && handleCreate()}
              />
              <motion.button
                className="btn-glow px-4 py-2 flex items-center gap-1 whitespace-nowrap"
                onClick={handleCreate}
                disabled={creating || !newBranchName.trim()}
                whileTap={{ scale: 0.97 }}
              >
                {creating ? <Loader2 size={13} className="animate-spin" /> : createSuccess ? <Check size={13} /> : <Plus size={13} />}
                {creating ? "Creating..." : createSuccess ? "Created!" : "Create"}
              </motion.button>
            </div>
          </motion.div>
        )}
      </AnimatePresence>

      {error && (
        <p className="text-xs mb-3 px-3 py-2 rounded-lg" style={{ background: "rgba(239,68,68,0.1)", color: "var(--accent-red)", border: "1px solid rgba(239,68,68,0.2)" }}>
          {error}
        </p>
      )}

      {/* Branch list */}
      <div className="space-y-2">
        {branches.length === 0 ? (
          <p className="text-xs text-center py-4" style={{ color: "var(--text-muted)" }}>
            No branches found
          </p>
        ) : (
          branches.map((branch, idx) => {
            const isActive = branch === currentBranch;
            const isLoading = checkingOut === branch;
            return (
              <motion.div
                key={branch}
                initial={{ opacity: 0, x: -8 }}
                animate={{ opacity: 1, x: 0 }}
                transition={{ delay: idx * 0.05 }}
                className="flex items-center justify-between px-3 py-2 rounded-lg cursor-pointer"
                style={{
                  background: isActive ? "rgba(99,102,241,0.1)" : "rgba(15,18,32,0.5)",
                  border: `1px solid ${isActive ? "var(--border-bright)" : "var(--border-subtle)"}`,
                  transition: "all 0.15s",
                }}
                onClick={() => handleCheckout(branch)}
                whileHover={!isActive ? { backgroundColor: "rgba(99,102,241,0.05)" } : {}}
              >
                <div className="flex items-center gap-2">
                  <GitBranch size={13} style={{ color: isActive ? "var(--accent-secondary)" : "var(--text-muted)" }} />
                  <span className="text-sm font-medium" style={{ color: isActive ? "var(--accent-secondary)" : "var(--text-primary)" }}>
                    {branch}
                  </span>
                  {isActive && <span className="badge badge-indigo text-[10px]">active</span>}
                </div>
                {!isActive && (
                  isLoading ? (
                    <Loader2 size={13} className="animate-spin" style={{ color: "var(--text-muted)" }} />
                  ) : (
                    <ArrowRight size={13} style={{ color: "var(--text-muted)", opacity: 0.5 }} />
                  )
                )}
              </motion.div>
            );
          })
        )}
      </div>
    </div>
  );
}
